#include "LFS.h"
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
int main(int argc, char * argv[]){
	
	if(argc < 2){
		cout << "USAGE " << argv[0] << " <input-filename>" << endl;
		return 1;
	}
	cout << "Enter number of segments: ";
	int numSegs;
	cin >> numSegs;
	cout << endl;

	cout << "Enter number of blocks per segment: ";
	int blocksPerSeg;
	cin >> blocksPerSeg;
	cout << endl;

	string file(argv[1]);
	ifstream infile(file);
	string line, word;
	getline(infile, line);
	stringstream s1(line);
	int numFiles;
	s1 >> numFiles;
	
	LFS sim(numFiles, numSegs, blocksPerSeg, POLICY_THREADING);

	while(getline(infile, line)){
		stringstream ss(line);
		ss >> word;
		int file_id, block_num;
		ss >> file_id;
		ss >> block_num;
		if(word == "WRITE"){
			sim.writeSingleBlock(file_id, block_num);
		}
		else if(word == "READ"){
			sim.read(file_id, block_num);	
		}
	}		

	// Testing clean
/*	sim.writeManyBlocks(1, 3);
	sim.writeManyBlocks(2, 3);
	sim.writeManyBlocks(3, 3);
	sim.writeSingleBlock(1, 1);
	sim.writeSingleBlock(2, 1);
	sim.writeSingleBlock(3, 1);
	sim.displayFSContents();
	sim.clean()
*/	
	// Display file system contents
	sim.displayFSContents();

	// Display file block to segment map
	sim.displayMap();
	cout << "Number of seeks: " << sim.getSeeks() << endl;
	return 0;
}
