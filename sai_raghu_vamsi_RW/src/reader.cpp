/* Mapper.c by Sai Raghu Vamsi Anumula */
#include <iostream>
#include <fstream>
#include <string.h>
#include "messaging.h"
#include <climits>

using namespace std;

int main(int argc, char* argv[])
{
    
    string line;
    char opt;
    char *temp;
    //init_messaging();
    while(opt != 'q')
    {
        cout << "INFO > NUM OF MESSAGES: " << get_msg_count() << endl;

        cout << "(r) Read a message" << endl;
        cout << "(p) Print buffer stats" << endl;
        cout << "(q) Quit program \n" << endl; 
        
        cout << "Enter an option: ";
        scanf("%c",&opt);
        cin.ignore(INT_MAX, '\n');
        //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'

         // print buffer stats
        switch(opt) 
        {
            case 'p':  
                print_buf_info();
                break;
                // write message
            case 'r':
                temp = read_message();

                if(temp == NULL)
                {
                    printf("\nRESULT > NO MESSAGE TO READ\n");
                    break;
                }
                printf("\nRESULT > RECEIVED MESSAGE: %s\n",temp);
                break;
                
            case 'q':
                break;

        }
       
    }
    
   // exit_messaging();

return 0;
}
/* 
Reader
*/

