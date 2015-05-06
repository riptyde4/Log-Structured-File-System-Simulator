#include "LFS.h"

int main(int argc, char * argv[]){

	int numFiles = 4;
	int numSegs = 10;
	int blocksPerSeg = 3;
	LFS sim(numFiles, numSegs, blocksPerSeg, POLICY_THREADING);

	// Test adding files
	sim.writeManyBlocks(1, 6);
	sim.writeManyBlocks(2, 10);
	sim.writeManyBlocks(3, 5);

	// Test updating existing blocks
	sim.writeSingleBlock(1, 5);
	sim.writeSingleBlock(2, 4);

	// Test writing new single blocks
	sim.writeSingleBlock(4, 1);
	sim.writeSingleBlock(4, 2);
	sim.writeSingleBlock(4, 3);

	// Display file system contents
	sim.displayFSContents();

	// Display file block to segment map
	sim.displayMap();

	return 0;
}
