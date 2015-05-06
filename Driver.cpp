#include "LFS.h"

int main(int argc, char * argv[]){

	int numFiles = 3;
	int numSegs = 10;
	int blocksPerSeg = 3;
	LFS sim(numFiles, numSegs, blocksPerSeg, POLICY_THREADING);

	// Test adding files
	sim.addFile(1, 6);
	sim.addFile(2, 10);
	sim.addFile(3, 5);

	// Test updating files
	sim.updateFile(1, 5);

	// Display file system contents
	sim.displayFSContents();

	// Display file block to segment map
	sim.displayMap();

	return 0;
}
