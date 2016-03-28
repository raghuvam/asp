/* Mapper.c by Sai Raghu Vamsi Anumula */
#include <iostream>
#include <fstream>
#include "messaging.h"
#include <string.h>
#include <climits>

using namespace std;

int main(int argc, char* argv[])
{
    char opt;
    string line;
    
    //ninit_messaging();
    while(opt != 'q')
    {
        cout << "(w) Write to buffer" << endl;
        cout << "(p) Print buffer stats" << endl;
        cout << "(q) Quit program \n" << endl; 
        cout << "Enter an option: ";
        scanf("%c",&opt);
        cin.ignore(INT_MAX, '\n');
        char *msg;
        // print buffer stats
        switch(opt) 
        {
            case 'w':
                cout << "Enter a message: " << endl;
                std::getline(cin,line);
                msg = new char[line.size()];
                strcpy(msg,line.c_str());
                if(!send_message(msg))
                    printf("RESULT > BUFFER FULL - MESSAGE NOT SENT\n" );
                else
                    printf("RESULT > MESSAGE SENT\n");
                break;

            case 'p':  
                print_buf_info();
                break;
                // write message
            
            default:
                break;
        }
        
  
    }
    
   // exit_messaging();

return 0;
}
