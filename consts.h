#ifndef CONSTS_H
#define CONSTS_H

#define SPACE '.'               // Space character
#define NEWCO '+'               // New company character
#define STAR '*'                // Star character
#define BLACKHOLE '@'           // Black hole character
#define NUMCO 5                 // Number of companies (don't change)
#define INIT_CO_COST 100        // Initial company start cost
#define INIT_CASH 6000          // Initial player cash
#define SPLIT_PRICE 3000        // When stocks split 2-1
#define FOUNDER_BONUS 5         // Founder gets this much stock
#define STARCOST 500            // Company's price increase near star
#define BLACKHOLECOST -500      // Price increase near black hole
#define NEWCOCOST 100           // Company's price increase near new co
#define NUMMOVES 5              // Number of different moves a player gets
#define MAXPLAYERS 4            // Total number of players a game can have
#define END_PERCENT 54          // End when this much of the map is full
#define DEF_SCREEN_LINES 25     // Default number of lines on screen
#define DEF_SCREEN_COLUMNS 80   // Default number of columns on screen

#define CR 13                   // Various keys
#define LF 10
#define BS 8
#define DEL 127
#define ESC 27
#define CTRL_L 12
#define CTRL_C 3
#define CTRL_Z 26

#endif
