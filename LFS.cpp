#include "LFS.h"

using namespace std;

void LFS::addFile(int fileID, int blocksInFile){
	int blocksFilled = 0;
	int currSegment = rw_head;

	// Initialize the fileID -> vector link in the imap
	vector<int> * forimap = new vector<int>;
	imap[fileID] = forimap;

	// Loop until all blocks of the file are stored
	while(blocksFilled < blocksInFile){
		// Make sure we are accessing a valid segment
		bool segmentAdded = false;
		if(currSegment < numSegments){
			// Check if the segment is full
			if(data[currSegment].live_blocks < blocksPerSegment){
				data[currSegment].live_blocks++;
				data[currSegment].free_blocks--;
				blocksFilled++;
				if(!segmentAdded){
					imap[fileID]->push_back(currSegment);
					segmentAdded = true;
				}
			}
			// If it's full, try the next segment
			else{
				currSegment++;
				segmentAdded = false;
			}
		}
		else{
			// Temporary loop termination
			cout << "Reached end of disk" << endl;
			break;

			// We have reached the end of the disk, need to figure out what to do here
			// Clean?
			// Go back and find a free block?
			// Exit the loop and display an error?
		}
	}

	// Update r/w head position
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

	// Add the new updated block

	// Update the imap
}