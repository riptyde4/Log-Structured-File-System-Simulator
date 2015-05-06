#include "LFS.h"
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
int main(int argc, char * argv[]){
	if(argc < 2){
		cout << "USAGE " << argv[0] << " <input-filename>" << endl;
		exit(1);
	}
	string file(argv[1]);
	ifstream infile(file);
	string line, word;
	getline(infile, line);
	stringstream s1(line);
	int numFiles;
	s1 >> numFiles;
	
	int numSegs = 10;
	int blocksPerSeg = 10;
	LFS sim(numFiles, numSegs, blocksPerSeg, POLICY_THREADING);

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
	}		
	
	// Display file system contents
	sim.displayFSContents();

	// Display file block to segment map
	sim.displayMap();

	return 0;
}
