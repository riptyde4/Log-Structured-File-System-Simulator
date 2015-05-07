#include "LFS.h"
#include <algorithm>

using namespace std;

// Constructor
LFS::LFS(int n, int s, int b, int p): 
numFiles(n), numSegments(s), blocksPerSegment(b), rw_head(1), policy(p){
	for(int i = 0; i <= numSegments; i++){
		Segment s(blocksPerSegment, numFiles);
		data.push_back(s);
	}

	// Initialize the file_block2segment map
	// Index 0 is a garbage map to allow for 1 indexing
	for(int fileID = 0; fileID <= numFiles; fileID++){
		unordered_map<int, int> blankMap;
		file_block2segment.push_back(blankMap);
	}
}   

void LFS::writeManyBlocks(int fileID, int blocksInFile){

	// Validate file ID
	if(fileID < 1 || fileID > numFiles){
		cerr << "Failed to add file ID " << fileID << ": file ID must satisfy 1 <= fileID <= numFiles." << endl;
		return;
	}

	int blocksFilled = 1;
	// Start the loop at the position of the r/w head
	int currSegment = rw_head;

	// Loop until all blocks of the file are stored
	bool segmentAdded = false;

	// This will track the number of times the r/w head has started
	// from the beginning of the disk. If the r/w head was half way through
	// the disk and reached the end, it will start at the beginning. If the
	// second time around, it still does not find a place to write the remaining
	// blocks(s), there is no point in continuing, because the entire disk is filled.
	// This count should never reach a value greater than 1.
	int restartCount = 0;

	while(blocksFilled <= blocksInFile && restartCount < 2){
		// Make sure we are accessing a valid segment
		if(currSegment < numSegments){
			// Check if the segment is full
			if(data[currSegment].live_blocks < blocksPerSegment){
				data[currSegment].live_blocks++;
				data[currSegment].free_blocks--;
				data[currSegment].block2file[fileID]++;
				// Add this segment to the file2block mapping
				file_block2segment[fileID][blocksFilled] = currSegment;
				blocksFilled++;
			}
			else{
				// If it's full, try the next segment
				currSegment++;

				// Update r/w head position
				rw_head = currSegment;
			}
		}
		else{
			endOfDiskHandler();
			restartCount++;
		}
	}

	if(restartCount == 2){
		cout << "Write of file ID " << fileID << " failed: "
		<< "disk is full. Blocks written: " << blocksFilled << endl;
	}
}

void LFS::writeSingleBlock(int fileID, int numBlock){

	// Validate file ID
	if(fileID < 1 || fileID > numFiles){
		cerr << "Failed to add file ID " << fileID << ": file ID must satisfy 1 <= fileID <= numFiles." << endl;
		return;
	}

	// Is the block already on disk?
	unordered_map<int, int>::const_iterator it = file_block2segment[fileID].find(numBlock);
	if(it != file_block2segment[fileID].end()){

		// Get the segment number
		int sNum = file_block2segment[fileID][numBlock];

		// Get the segment from disk
		Segment * s;
		if(sNum < numSegments){
			s = &data[sNum];
		}

		// Invalidate the original block
		s->free_blocks++;
		s->live_blocks--;
		s->block2file[fileID]--;
	}

	// Add the new updated block
	// Start the loop at the position of the r/w head
	int currSegment = rw_head;
	// Make sure we are accessing a valid segment
	bool blockWritten = false;
	int restartCount = 0;
	while(!blockWritten && restartCount < 2){
		if(currSegment < numSegments){
			// Check if the segment is full
			if(data[currSegment].live_blocks < blocksPerSegment){
				// Update the segment characteristics
				data[currSegment].live_blocks++;
				data[currSegment].free_blocks--;
				data[currSegment].block2file[fileID]++;

				// Update the file block to segment mapping
				file_block2segment[fileID][numBlock] = currSegment;

				// Set the flag to terminate the loop
				blockWritten = true;
			}
			else{
				// If it's full, try the next segment
				currSegment++;
				// Update rw head
				rw_head = currSegment;
			}
		}
		else{
			endOfDiskHandler();
			restartCount++;
		}
	}

	if(restartCount == 2){
		cerr << "Update to file ID " << fileID << " failed: "
		<< " the disk is full." << endl;
	}
}

void LFS::clean(){
	// Do while there could still be enough cleanable blocks to fill a segment
	bool blocksToClean = true;
	while(blocksToClean){
		// Find blocks in unfilled segments and add them to the segmentsToClean vector
		// If the accumulated total of blocks adds up to blocksPerSegment, move them
		// Should we move them even if the total of blocks does not completely fill a new segment?
		// My assumption is no, because this will simply just create another block that has to be 
		// cleaned later on. Note that not cleaning all blocks from a segment will result in the same,
		// but this scenario is handled by checking if there are still blocks left in the segment
		// and then later backtracking to that segment, moving it's remaining blocks.
		vector<int> segmentsToClean;
		unordered_map<int, int> blocksFromSegment;
		int accumulatedBlocks = 0;
		for(int currSegment = 1; currSegment <= numSegments; currSegment++){
			if(data[currSegment].live_blocks > 0 && data[currSegment].live_blocks < blocksPerSegment){
				// If not all of the blocks are occupied, then this segment needs to be cleaned
				segmentsToClean.push_back(currSegment);
				// Only take enough blocks to fill up a segment
				int blocksLeft = data[currSegment].live_blocks;
				int blocksTaken = 0;
				while((accumulatedBlocks + blocksTaken) < blocksPerSegment && blocksLeft > 0){
					blocksLeft--;
					blocksTaken++;
				}
				accumulatedBlocks += blocksTaken;
				blocksFromSegment[currSegment] = blocksTaken;

				// If we have not taken all of the blocks from the current segment, it now needs
				// to be cleaned as well, so decrement currSegment so the next iteration will consider it
				// Note that the only reason blocksLeft would be greater than 0 is that we have reached
				// our desired amount of accumulated blocks, so this should not mess anything up.
				if(blocksLeft > 0){
					currSegment--;
				}

			}
			if(accumulatedBlocks == blocksPerSegment){
				// Choose a clean segment to fill with the accumulated blocks
				// May need to add handling for the case that there are no clean segments
				// However, this should not happen (i think..)
				int cleanSegment = 1;
				for(int currSegment0 = 1; currSegment0 <= numSegments; currSegment0++){
					if(data[currSegment0].free_blocks == blocksPerSegment){
						cleanSegment = currSegment0;
						break;
					}
				}

				// Update the characteristics of the new segment that will hold these blocks (move them to cleanSegment)
				data[cleanSegment].free_blocks = 0;
				data[cleanSegment].live_blocks = accumulatedBlocks;

				// Update the characteristics of the segments (remove the blocks from them)
				// Don't update file2block yet, still need that data to speed up lookups - will do later
				for(auto it : segmentsToClean){
					int segmentToUpdate = it;
					//data[segmentToUpdate].free_blocks += data[segmentToUpdate].live_blocks;
					data[segmentToUpdate].free_blocks += blocksFromSegment[segmentToUpdate];
					data[segmentToUpdate].live_blocks -= blocksFromSegment[segmentToUpdate];

					// Update the map for each of the segments
					// For each of the files that have at least 1 block in this segment:
					// 		1. Look up which block(s) this file has in currSegment in the map
					// 		2. Update the map to show that these blocks are no longer in currSegment but now in cleanSegment 
					int blocksToRemove = blocksFromSegment[segmentToUpdate];
					for(int fileID = 1; fileID <= data[segmentToUpdate].block2file.size() && blocksToRemove > 0; fileID++){
						int blocks = data[segmentToUpdate].block2file[fileID];
						if(blocks > 0){
							for(int b = 1; b <= file_block2segment[fileID].size() && blocksToRemove > 0; b++){
								if(file_block2segment[fileID][b] == segmentToUpdate){
									file_block2segment[fileID][b] = cleanSegment;
									// Update the block to file map for each segment that had blocks removed from it
									// to reflect the fact that no files have any blocks in these segments any longer
									data[segmentToUpdate].block2file[fileID]--;
									// Update the clean segments map to show that the files' blocks now reside there
									data[cleanSegment].block2file[fileID]++;
									blocksToRemove--;
								}
							}
						}
					}
				}
				blocksFromSegment.clear();
				segmentsToClean.clear();
				accumulatedBlocks = 0;
			}
		}
		// If the for loop did not accumulate enough blocks to fill a new segment, terminate the loop
		if(accumulatedBlocks < blocksPerSegment){
				blocksToClean = false;
		}
	}	
}

void LFS::endOfDiskHandler(){
	cout << "Reached end of disk." << endl;
	// We have reached the end of the disk, need to figure out what to do here
	// Clean?
	// Go back and find a free block?
	// Exit the loop and display an error?

	// Threading approach - Start at the beginning of the log
	// This approach will cause fragmentation of the log
	if(policy = POLICY_THREADING){
		rw_head = 1;
	}

	// Compaction approach with threading
	else if(policy = POLICY_CLEAN){
		// Move free blocks from unfilled segments to a new segment, leaving clean segments
		clean();
		// Start at the beginning of the disk
		rw_head = 1;
	}

}

void LFS::displayFSContents(){
	cout << "==== BEGIN DISPLAY OF FILE SYSTEM CONTENTS ====" << endl;
	for(int segment = 1; segment < numSegments; segment++){
		cout << "Segment " << segment << " : " 
		<< "Free Blocks: " << data[segment].free_blocks 
		<< "\tLive Blocks: " << data[segment].live_blocks 
		<< endl;
		for(auto it : data[segment].block2file){
			if(it.first != 0){
				cout << "\tFile ID " << it.first << " has " << it.second
				<< " blocks in this segment." << endl;
			}
		}
	}
	cout << "==== END DISPLAY OF FILE SYSTEM CONTENTS ====" << endl;
}

void LFS::displayMap(){

	cout << "==== BEGIN DISPLAY OF FILE BLOCK TO SEGMENT MAP ====" << endl;
	for(int id = 1; id <= numFiles; id++){
		for(auto it0 : file_block2segment[id]){
			cout << "File ID " << id << " block " << it0.first 
			<< " is located in segment " << it0.second << endl;
		}
	}
	cout << "==== END DISPLAY OF FILE BLOCK TO SEGMENT MAP ====" << endl;
}

