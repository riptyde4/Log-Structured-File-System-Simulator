#include "LFS.h"
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
int main(int argc, char * argv[]){
	/*
	if(argc < 2){
		cout << "USAGE " << argv[0] << " <input-filename>" << endl;
		return 1;
	}
	string file(argv[1]);
	ifstream infile(file);
	string line, word;
	getline(infile, line);
	stringstream s1(line);
	int numFiles;
	s1 >> numFiles;
	*/
	
	int numFiles = 4;
	int numSegs = 20;
	int blocksPerSeg = 3;
	LFS sim(numFiles, numSegs, blocksPerSeg, POLICY_THREADING, 66);
/*
	while(getline(infile, line)){
		stringstream ss(line);
		ss >> word;
		if(word == "WRITE"){
			int file_id, block_num;
			ss >> file_id;
			ss >> block_num;
			sim.writeSingleBlock(file_id, block_num);
		}
		else if(word == "READ"){
			;
		}
	}		*/

	// Testing clean
	sim.writeManyBlocks(1, 3);
	sim.writeManyBlocks(2, 3);
	sim.writeManyBlocks(3, 3);
	sim.writeManyBlocks(4, 3);

	sim.displayFSContents();
	sim.displayMap();

	cout << endl << "Updating some blocks..." << endl;

	sim.writeSingleBlock(1, 1);
	sim.writeSingleBlock(2, 1);
	sim.writeSingleBlock(3, 1);
	sim.writeSingleBlock(3, 2);
	sim.writeSingleBlock(2, 1);
	sim.writeSingleBlock(4, 1);

	sim.displayFSContents();
	sim.displayMap();
	cout << endl;
	sim.clean();

	
	// Display file system contents

	sim.displayFSContents();

	// Display file block to segment map

	sim.displayMap();

	return 0;
}
