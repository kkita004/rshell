#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int file_exist(char *filename){
	struct stat buffer;
	return(stat(filename, &buffer) == 0);
}

int main(int argc, char **argv){
	if(argc != 3) {
		std::cerr << "Usage: mv <file1> <file2>" << std::endl;
		exit(1);
	}
	
	std::string file1(argv[1]);
	std::string file2(argv[2]);
	struct stat st;
	if(!file_exist(argv[1])){
		std::cerr << "file to copy " << file1 << "does not exist." << std::endl;
		exit(1);
	}
	if(!(file_exist(argv[2]))){
		if(-1 == link(file1.c_str(), file2.c_str())){
			perror("link");
			exit(1);
		}
		if(-1 == unlink(file1.c_str())){
			perror("unlink");
			exit(1);
		}
	}
	else{
		if(-1 == stat(file2.c_str(), &st)){
			perror("stat");
			exit(1);
		}
		if(st.st_mode & S_IFDIR){
			std::string path = file2 + "/" + file1;
			if(-1 == link(file1.c_str(), path.c_str())){
				perror("link");
				exit(1);
			}
			if(-1 == unlink(file1.c_str())){
				perror("unlink");
				exit(1);
			}
		}
		else{
			std::cerr << "file " << file2 << " already exist." << std::endl;
		}
	}
	return 0;
}
