#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <sys/time.h>
using namespace std;

struct file{
	int size;
	int id;
};

vector<struct file> generateFiles(int num_files, int file_size, int file_variety){
	vector<struct file> file_vector;
	for(int i = 0; i < file_size; i++){
		if(file_variety = 0){
			struct file f1= {file_size, i+1};
			file_vector.push_back(f1);
		}
		else{
			struct file f1 = {rand() % file_size + 1, i+1};
			file_vector.push_back(f1);
		}
	}
	return file_vector;	
}
void generateHighLocality(vector<struct file> files, int rw_mix, int num_references){
	ofstream outfile("generated_output.txt");
	int repeat = num_references;
	int last_file_accessed = -1;
	while(repeat){
		bool write = rand() % 100 >= rw_mix? 1 : 0;
		int file_num = rand() % files.size();
		if(last_file_accessed == -1){
			last_file_accessed = file_num;
		}
		else{
			if(rand() % 100 >= 90){ //10% chance to avoid locality of reference and start referencing new file
				last_file_accessed = file_num;
			}
		}
		if(write){
			int block_number = rand() % files[last_file_accessed].size + 1;
			outfile << "WRITE " << files[last_file_accessed].id << " " << block_number << endl;
		}
		else{
			int block_number = rand() % files[last_file_accessed].size + 1;
			outfile << "READ " << files[last_file_accessed].id << " " << block_number << endl;
		}
		repeat--;
	}
	outfile.close();
}
void generateLowLocality(vector<struct file> files, int rw_mix, int num_references){
	ofstream outfile("generated_output.txt");
	int repeat = num_references;
	while(repeat){
		bool write = rand() % 100 >= rw_mix? 1 : 0;
		int file_num = rand() % files.size();
		if(write){
			int block_number = rand() % files[file_num].size + 1;
			outfile << "WRITE " << files[file_num].id << " " << block_number << endl;
		}
		else{
			int block_number = rand() % files[file_num].size + 1;
			outfile << "READ " << files[file_num].id << " " << block_number << endl;
		}
		repeat--;
	}
	outfile.close();
}
void generateOutput(vector<struct file> files, int degree_locality, int rw_mix, int num_references){
	int num_files = files.size();
	if(degree_locality == 0){
		generateHighLocality(files, rw_mix, num_references);
	}
	else{
		generateLowLocality(files, rw_mix, num_references);
	}
}
int main(int argc, char** argv){
	cout << "Specify number of files: " << endl;
	int num_files = 0;
	cin >> num_files;
	cout << "Specify largest file size: " << endl;
	long int file_size = 0;
	cin >> file_size;
	cout << "Uniform (0) or Varied (1) file size?" << endl;
	int file_variety = 1;
	cin >> file_variety;
	cout << "Specify number of references: " << endl;
	int num_references;
	cin >> num_references;
	cout << "Specify degree of locality of reference [0 - high locality ,  1 - no locality]" << endl;
	int degree_locality = 0;
	cin >> degree_locality;
	cout << "Specify percentage of reads to writes: [give an integer value between 0 and 100]" << endl;
	int rw_mix = 0;
	cin >> rw_mix;
	cout << "Given input, generating output file to generated_output.txt" << endl;
	
	srand(time(NULL));
	
	vector<struct file> files = generateFiles(num_files, file_size, file_variety);	
	generateOutput(files, degree_locality, rw_mix, num_references);
}

