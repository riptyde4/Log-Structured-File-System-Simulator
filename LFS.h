#include <iostream>
#include <vector>
#include <unordered_map>

#define POLICY_THREADING 1
#define POLICY_CLEAN 2

#define BLOCK_NOT_ON_DISK 10

class LFS{

	// Tracking
	int totalReads;
	int totalWrites;
	int totalSeeks;
	int rw_head;

	// Limits
	int numSegments;
	int blocksPerSegment;
	int numFiles;

	// Reset age to 0 on every overwrite
	// Increment all other segment ages by 1
	// Low age = Hot segment
	// High age = Cold segment
	class Segment{
		public:
	   		int age;
	    	int live_blocks;
	    	int free_blocks;
	    	// This map will store how many blocks a file has
	    	// in this segment - used to prevent accidental deletion
	    	std::unordered_map<int, int> block2file;
			Segment(int numBlocks, int num_Files){
				free_blocks = numBlocks;
				live_blocks = 0;
	    	}
	};

	// The physical disk
	std::vector<Segment> data;
	int policy;

	// This vector of maps will map each file id & it's block number
	// to the segment that the block was stored in
	// The index of the vector is the file ID
	// The map key is the block number, the value is the segment number
	std::vector<std::unordered_map<int, int>> file_block2segment;

	public:
    	// Constructor
	// n = num files
	// s = num segments
	// b = blocks per segment
	// p = policy
	LFS(int n, int s, int b, int p);

    	// Functions
	void writeManyBlocks(int fileID, int blocksToWrite);
	void writeSingleBlock(int fileID, int numBlock);
    	void endOfDiskHandler();
    	void clean(); // ???
    	void displayFSContents();
	void displayMap();
};
