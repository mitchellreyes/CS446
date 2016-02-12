#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

// Forward Declarations
int setData( const char*, char[] );
int findFamily( char[], int, int );
int findSpouse( char[], int );

int main( int argc, char* argv[] )
{
    /*Initialize family array*/
    char family[1000];
    
    /*get data from the CL file*/
    /*if error in the CL*/
    if( setData( argv[1], family ) == 1 )
    {
        /*exit the program*/
        exit(1);
    }
    /*counter to keep track of tabs*/
    int tabCount = 0;
    /*current index of the array*/
    int currentIndex = 0;
    /*print the first couple*/
    fprintf( stdout, "%c-%c\n", family[ currentIndex ], family[currentIndex+1] );
    /*set start index*/
    currentIndex = 2;
    tabCount++;

    /*Create the family tree*/
    if( findFamily( family, tabCount, currentIndex ) == 1 )
    {
        fprintf( stderr, "Unable to create family tree.\n" );
        exit(1);
    }
    return 0;
}

/*
@stores all the data from a file into the array
@also handles any errors that occur with file I/O
 */
int setData( const char* fileName, char data[] )
{
    /*file pointer and open it*/
    FILE* doc;
    doc = fopen( fileName, "r" );

    /*if you can open/find the file*/
    if( doc == NULL )
    {
        fprintf( stderr, "Unable to open file. \n" );
        exit(1);
    }
    /*storing the data into the array*/
    else
    {
        /*index pointer for the file*/
        int currentIndex = 0;
        /*keep reading until it hits the end of the file*/
        while(!feof(doc))
        {
            /*putting a char into each element of the array*/
            fscanf( doc , "%s", &data[ currentIndex ] );
            currentIndex++;

            /*fail safe in case file too big*/
            if(currentIndex > 1000)
            {
                fprintf(stderr, "File too large.\n");
                exit(1);
            }
        }
        /*put a null char at the end of the array*/
        data[ currentIndex ] = '\0';
    }

    /*Close the file after succesful reading*/
    fclose( doc );
    return 0;
}

/*
@goes through the family array and finds the spouses
@forks if you found a spouse
*/
int findFamily( char family[], int tabCount, int currentIndex )
{
    /*set the number of children*/
    int numChild = ( family[ currentIndex ] - '0' );
    currentIndex++;

    /*Check every child*/
    int count = 0;
    for( count = 0; count < numChild; count++ )
    {
        /*check if spouse*/
        int spouseLocation = findSpouse( family, currentIndex );

        /*case: spouse exists*/
        if( spouseLocation > -1 )
        {
            /*fork to handle more children*/
            pid_t pid; 
            pid = fork();

            /*child*/
            if( pid == 0 )
            {
                for( count = 0; count < tabCount; count++ )
                {
                    fprintf( stdout, "\t" );
                }

                /*print out the couple together (i.e A-B)*/
                fprintf( stdout, "%c-%c\n", family[ currentIndex ], family[ spouseLocation ] );
                /*see the couple has children*/
                findFamily( family, tabCount + 1, spouseLocation + 1 );
                return 0;
            }
            /*parent*/
            else if( pid > 0 )
            {
                /*wait for all child processes to die*/
                waitpid( pid, NULL, 0 );
            }
            else /*(pid < 0)*/
            {
                /*error in the forking process*/
                fprintf( stderr, "fork() error***\n");
                return 1;
            }
        }
        /*case: no spouse*/
        else
        {
            /*print out tabs*/
            int tabs;
            for( tabs = 0; tabs < tabCount; tabs++ )
            {
                fprintf( stdout, "\t" );
            }
            /*print out the single and ready to mingle man*/
            fprintf( stdout, "%c\n", family[ currentIndex ] );
        }

        /*check the next person*/
        currentIndex++;
    }
    return 0;
}

/*
@finds the spouse from a given index
@goes through the whole array
 */
int findSpouse( char family[], int currentIndex )
{
    /*setting it so that youre checking everyone*/
    int count = 0;
    /*search the whole family for it*/
    while( family[ count ] != '\0' )
    {
        /*if you find a spouse, return the index*/
        if( family[ count ] == family[ currentIndex ] && count != currentIndex )
        {
            return ( count + 1 );
        }
        count++;
    }
    /*if there is no spouse to find*/
    return -1;
}