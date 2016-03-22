/* Mapper.c by Sai Raghu Vamsi Anumula */
#include <iostream>
#include <stdlib>


using namespace std;

/*
class message
{
    public:
        char text[256];

    private:

};
*/

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
		// Check if the input file is given as argument to the mapper
    	perror("Give Input filename as argument");
    	exit(0);
    }
    
    char line[100];
    FILE * fh;
    fh = fopen(argv[1],"r");
    
    while(1)
    {
    	// read each line frm the input file.
        fscanf(fh,"%s",line);
        if(!feof(fh)) // exit the while loop if eof is reached.
            cout << "(" << line <<",1)" << endl; // print out the string in (word,1) format
        else
            break;
    }
    close(fh); // close input file descriptor once printing is done
return 0;
}
