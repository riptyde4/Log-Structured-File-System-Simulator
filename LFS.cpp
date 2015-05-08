#include "LFS.h"
#include <algorithm>

using namespace std;

// Constructor
LFS::LFS(int n, int s, int b, int p, int u): 
numFiles(n), numSegments(s), blocksPerSegment(b), rw_head(1), policy(p), utilThreshold(u){
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
	totalSeeks = 0;
	sizeAdded = 0;
	totalSize = numSegments * blocksPerSegment;
}   
int LFS::getSeeks(){
	return totalSeeks;
}
void LFS::writeManyBlocks(int fileID, int blocksInFile){

	// Validate file ID
	if(fileID < 1 || fileID > numFiles){
		cerr << "Failed to add file ID " << fileID << ": file ID must satisfy 1 <= fileID <= numFiles." << endl;
		return;
	}

	int blocksFilled = 1;
	// Start the loop at the position of the r/w head
	//int currSegment = rw_head;
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
	if(sizeAdded == totalSize){
		cout << "DISK IS FULL" << endl;
		exit(1);
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
//	int currSegment = rw_head;
	int currSegment = rw_head;
	// Make sure we are accessing a valid segment
	bool blockWritten = false;
	int restartCount = 0;
	int movedSegments = 0;
	while(!blockWritten){
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
				int moveThreshold = numSegments/2;
				if(movedSegments < moveThreshold){
					currSegment++;
					movedSegments++;

					// Update rw head
					rw_head = currSegment;
				}
				// If we've moved too many times, clean and pick the next segment
				else{
					// If it's full, clean
					clean();
					int cleanSegment = 0;				
					// Take the next clean segment from the list
					for(int i = 0; i < utilizationList.size(); i++){
						if(utilizationList[i].second == 0){
							cleanSegment = utilizationList[i].first;
						}
					}
					currSegment = cleanSegment;
					rw_head = currSegment;
				}

				// If we moved to the next segment, we've seeked
				// If we moved to the next cleanSegment after cleaning, we've also seeked
				totalSeeks++;
			}
		}
		else{
			break;
		}
	}
	sizeAdded++;
	if(restartCount == 2){
		cerr << "Update to file ID " << fileID << " failed: "
		<< " the disk is full." << endl;
		endOfDiskHandler();
	}
}
void LFS::read(int fileID, int numBlock){

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

		// Read the block
		rw_head = sNum;
		totalSeeks++;
	}
	else{
		writeSingleBlock(fileID, numBlock);
	}
}
bool utilCompare(pair<int, float> p1, pair<int, float> p2){
	return p1.second < p2.second;
}
// Seek when:
// write from one segment to another
// move in a segment
void LFS::clean(){
	// Case 1: Some underutilized segments and at least 1 free segment
	//	-->
	int empty_segment = 0;
	for(int i = 0; i < utilizationList.size(); i++){
		if(utilizationList[i].second == 0){
			empty_segment = utilizationList[i].first;
		}
	}

	for(int i = 0; i < utilizationList.size(); i++){
		if(utilizationList[i].second > 0 && utilizationList[i].second < utilThreshold){
			for(int j = i+1; j < utilizationList.size(); j++){
				if(utilizationList[i].second + utilizationList[j].second <= 100){
					//compact them together into the free segment and resort the util list and run this again
					int blocksToRemove = data[utilizationList[i].first].live_blocks;
					for(int fileID = 1; fileID <= data[utilizationList[i].first].block2file.size() && blocksToRemove > 0; fileID++){
						int blocks = data[utilizationList[i].first].block2file[fileID];
						if(blocks > 0){
							for(int b = 1; b <= file_block2segment[fileID].size() && blocksToRemove > 0; b++){
								if(file_block2segment[fileID][b] == utilizationList[i].first){
									file_block2segment[fileID][b] = utilizationList[j].first;
									// Update the block to file map for each segment that had blocks removed from it
									// to reflect the fact that no files have any blocks in these segments any longer
									data[utilizationList[i].first].block2file[fileID]--;
									// Update the clean segments map to show that the files' blocks now reside there
									data[utilizationList[j].first].block2file[fileID]++;
									blocksToRemove--;
									totalSeeks++;
									data[utilizationList[i].first].live_blocks--;
									data[utilizationList[j].first].live_blocks++;
									data[utilizationList[i].first].free_blocks++;
									data[utilizationList[j].first].free_blocks--;
								}
							}
						}
					}	
				}
				else{
					//distribute i into j and on....
					int blocksToRemove = data[utilizationList[i].first].live_blocks;
					int leftOverBlocks = blocksToRemove - data[utilizationList[j].first].free_blocks;	
					for(int fileID = 1; fileID <= data[utilizationList[i].first].block2file.size() && blocksToRemove > leftOverBlocks; fileID++){
						int blocks = data[utilizationList[i].first].block2file[fileID];
						cout << "FILE ID IS LOOPING FOREVER YOU MOTHERFUCKERS" << endl;
						if(blocks > 0){
							for(int b = 1; b <= file_block2segment[fileID].size() && blocksToRemove > leftOverBlocks; b++){
								if(file_block2segment[fileID][b] == utilizationList[i].first){
									file_block2segment[fileID][b] = utilizationList[j].first;
									// Update the block to file map for each segment that had blocks removed from it
									// to reflect the fact that no files have any blocks in these segments any longer
									data[utilizationList[i].first].block2file[fileID]--;
									// Update the clean segments map to show that the files' blocks now reside there
									data[utilizationList[j].first].block2file[fileID]++;
									blocksToRemove--;
									totalSeeks++;
									data[utilizationList[i].first].live_blocks--;
									data[utilizationList[j].first].live_blocks++;
									data[utilizationList[i].first].free_blocks++;
									data[utilizationList[j].first].free_blocks--;
								}
							}
						}
					}
					//continue to next j
				}
				updateUtilizationList();
			}
		}	
	}
	

		
	// Case 2: Some underutilized segments and no free segments
	// 	-->Compact the underutilized segments blocks into other underutilized blocks (whichever ones they fit into)
}

void LFS::updateUtilizationList(){
	
	// Clear the current values
	utilizationList.clear();

	// Go through the disk, getting the utilizatation of each, and placing it into a pair with it's index;
	for(int i = 0; i <= numSegments; i ++){
		float utilization = ((float)data[i].live_blocks / (float)blocksPerSegment) * 100.0f;
		pair<int, float> p;
		p.first = i;
		p.second = utilization;
		utilizationList.push_back(p);
	}

	// Sort by the utilization
	sort(utilizationList.begin(), utilizationList.end(), utilCompare);

	// print test
	for(int i = 1; i < utilizationList.size(); i ++){
		cout << "ID: " << utilizationList[i].first << "\t Utilization: " << utilizationList[i].second << endl;
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
	for(int segment = 1; segment <= numSegments; segment++){
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

