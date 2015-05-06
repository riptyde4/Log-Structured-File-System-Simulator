#include "LFS.h"

using namespace std;

// Constructor
LFS::LFS(int n, int s, int b): 
numSegments(s), blocksPerSegment(b), rw_head(0){
	for(int i = 0; i < numSegments; i++){
		Segment s(blocksPerSegment);
		data.push_back(s);
	}

		// Initialize the fileID -> vector link in the imap
		for(int fileID = 0; fileID < numFiles; i++){
		vector<int> vec;
		imap[fileID] = vec;
	}
}   

void LFS::addFile(int fileID, int blocksInFile){
	int blocksFilled = 0;
	// Start the loop at the position of the r/w head
	int currSegment = rw_head;

	// Loop until all blocks of the file are stored
	bool segmentAdded = false;
	while(blocksFilled < blocksInFile){
		// Make sure we are accessing a valid segment
		if(currSegment < numSegments){
			// Check if the segment is full
			if(data[currSegment].live_blocks < blocksPerSegment){
				data[currSegment].live_blocks++;
				data[currSegment].free_blocks--;
				blocksFilled++;
				if(!segmentAdded){

					// Add this segment to the file's inode
					imap[fileID]->push_back(currSegment);

					// Set the segmentAdded flag true so that future 
					// iterations with the same value for currSegment
					// will not add the same segment twice
					segmentAdded = true;
				}
			}
			// If it's full, try the next segment
			else{
				currSegment++;

				// Since currSegment has now been changed, set the segmentAdded 
				// flag so that the next iteration will add the new segment
				segmentAdded = false;
			}
		}
		else{
			// Temporary loop termination
			cout << "Reached end of disk" << endl;
			endOfDiskHandler();
			break;
		}
	}

	// Update r/w head position
	// Note sure what this should be tracking exactly
	// it currently keeps track of the current segment
	rw_head = currSegment;
}

void LFS::updateFile(int fileID, int numBlock){
	// Check if the file is on disk
	unordered_map<int, vector<int>>::const_iterator it = imap.find(fileID);
	if(it == imap.end()){
		cerr << "Update to file " << fileID << " failed. File not on disk." << endl;
		return;
	}

	// Get segment number
	int sNum = numBlock % blocksPerSegment;

	// Get the segment
	Segment * s;
	if(sNum < numSegments){
		s = &data[sNum];
	}

	// Invalidate the original block
	s->free_blocks++;
	s->live_blocks--;

	// Remove the old segment from the file's inode
	imap[fileID].erase(std::find(imap.begin(), imap.end(), sNum);

	// Add the new updated block
	// Start the loop at the position of the r/w head
	int currSegment = rw_head;
	// Make sure we are accessing a valid segment
	if(currSegment < numSegments){
		// Check if the segment is full
		if(data[currSegment].live_blocks < blocksPerSegment){
			data[currSegment].live_blocks++;
			data[currSegment].free_blocks--;
		}
		// If it's full, try the next segment
		else{
			currSegment++;
		}
	}
	else{
		// Temporary loop termination
		cout << "Reached end of disk" << endl;
		endOfDiskHandler();
		break;
	}

	// Update the imap and rw head
	rw_head = currSegment;
	imap[fileID].push_back(currSegment);
}

void LFS::endOfDiskHandler(){
	// We have reached the end of the disk, need to figure out what to do here
	// Clean?
	// Go back and find a free block?
	// Exit the loop and display an error?
}

void LFS::displayDiskContents(){
	int totalBlocks = numSegments * blocksPerSegment;
	for(int block = 0; block < totalBlocks; block++){
		int numSegment = block % blocksPerSegment;
		cout << "Segment " << numSegment << " Block " << block
		<< " : " << "Free Blocks: " << data[block].free_blocks 
		<< "\tLive Blocks: " << data[block].live_blocks << endl;
	}
}