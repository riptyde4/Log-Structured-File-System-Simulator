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
		public:
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
    int numFiles;

    // The physical disk
    vector<Segment> data;

    // This will map files to their inodes
	// imap[fileid] = vector<int> inode;
	// inode vectors will store the segments holding 
	// the blocks associated with each file
	unordered_map<int, vector<int>> imap;

	public:
    	// Constructor
    	LFS(int n, int s, int b);

    	// Functions
   		void addFile(int fileID, int blocksInFile);
    	void updateFile(int fileID, int numBlock);
    	void endOfDiskHandler();
    	void clean(); // ???
    	void displayDiskContents();
};
