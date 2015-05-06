#include <iostream>
#include <vector>
#include <unordered_map>

#define POLICY_THREADING 1
#define POLICY_CLEAN 2

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
    std::vector<Segment> data;
    int policy;

    // This will map files to their inodes
	// imap[fileid] = vector<int> inode;
	// inode vectors will store the segments holding 
	// the blocks associated with each file
	std::unordered_map<int, std::vector<int>> imap;

	public:
    	// Constructor
    	LFS(int n, int s, int b, int p);

    	// Functions
   		void addFile(int fileID, int blocksInFile);
    	void updateFile(int fileID, int numBlock);
    	void endOfDiskHandler();
    	void clean(); // ???
    	void displayDiskContents();
    	void displayInodes();
};
