#include <iostream>
#include <vector>
#include <unordered_map>
#include "Segment.h"

class LFS{

	// Reset age to 0 on every overwrite
	// Increment all other segment ages by 1
	// Low age = Hot segment
	// High age = Cold segment
	class Segment{
	    int age;
	    int live_blocks;
	    int free_blocks;
	    Segment(int numBlocks){
	    	free_blocks = numBlocks;
	    	live_blocks = 0;
	    }
	};

	// Tracking
	int totalReads;
	int totalWrites;
	int totalSeeks;
	int rw_head;

	// Limits
    int numSegments;
    int blocksPerSegment;

    // The physical disk
    vector<Segment> data;

    // This will map files to their inodes
	// imap[fileid] = vector<int> inode;
	// inode vectors will store the segments holding 
	// the blocks associated with each file
	unordered_map<int, vector<int>*> imap;

    // Constructor
    LFS(int s, int b): 
    numSegments(s), blocksPerSegment(b), rw_head(0){
    	for(int i = 0; i < numSegments; i++){
    		Segment s(blocksPerSegment);
    		data.push_back(s);
    	}
    }   

    // Functions
    void addFile(int fileID, int blockInFile);
    void updateFile(int fileID);
    void clean(); // ???
    void displayDiskContents();
};
