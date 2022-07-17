#include "screen.h"
#include "ports.h"
#include "../kernel/util.h"


/* Declaration of private functions */
int get_cursor_offset();
void set_cursor_offset(int offset);
int print_char(char character, int col, int row, char attribute_byte);
int get_offset(int col, int row);
int get_offset_row(int offset);
int get_offset_col(int offset);
int handle_scrolling(int offset);

/**********************************************************
 * Public Kernel API functions                            *
 **********************************************************/

void clear_screen() {
    int screen_size = MAX_COLS * MAX_ROWS;
    char* screen = VIDEO_ADDRESS;

    for (int i = 0; i < screen_size; i++) {
        screen[i*2] = ' ';
        screen[i*2+1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(get_offset(0, 0));
}

/* Print a message on the specified location
 * If col, row, are negative, we will use the current offset
 */
void kprint_at(char* message, int col, int row) {
    int offset;
    if (col >= 0 && row >= 0) {
        offset = get_offset(col, row);
    } else {
        offset = get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    /* Loop through message and print it */
    int i = 0;
    while(message[i] != 0) {
        offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
        /* Compute row and col for next iteration */
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

void kprint(char* message) {
    kprint_at(message, -1, -1);
}


/**********************************************************
 * Private Kernel functions                               *
 **********************************************************/


/* Print a char on the screen at col, row, or at the cursor position 
 * Return the new offset 
 */
int print_char(char character, int col, int row, char attribute_byte) {
    /* Create a byte pointer to the video memory address */
    unsigned char* vidmem = (unsigned char*) VIDEO_ADDRESS;

    /* If attribute byte is 0, assume default style */
    if (!attribute_byte) {
        attribute_byte = WHITE_ON_BLACK;
    }

    /* Get video memeory offset for the screen location */
    int offset;
    /* If row and col are non-negative, use them for offset */
    if (col >= 0 && row >= 0) {
        offset = get_offset(col, row);
    /* Otherwise use cursor pos */
    } else {
        offset = get_cursor_offset();
    }

    /* If we see a newline character, set the offset to the end of the 
    current row, so it will be advanced to the first col of the next row */
    if (character == '\n') {
        int rows = offset / (2*MAX_COLS);
        offset = get_offset(MAX_COLS-1, rows);
    /* Otherwise write the character and its attribute byte
    to video memory at our calculated offset */
    } else {
        vidmem[offset] = character;
        vidmem[offset+1] = attribute_byte;
    }

    /* Update the offset to the next character cell, 2 bytes ahead */
    offset += 2;

    /* Make scrolling adjustment for when we reach bottom of screen */
    offset = handle_scrolling(offset);

    /* Update the cursor position */
    set_cursor_offset(offset);

    return offset;
}

/* Use the VGA ports to get the current cursor position
1. Ask for high byte of the cursor offset (data 14)
2. Ask for low byte (data 15)
 */
int get_cursor_offset() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; /* High byte: << 8 */
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2; /* Position * size of cell */
}

/* Similar to get_cursor_offset, but instead of reading we write data */
void set_cursor_offset(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

/* Check if the offset is over screen size and scroll */
int handle_scrolling(int offset) {
    if (offset >= (MAX_COLS * MAX_ROWS * 2)) {
        /* Copy all rows to the address of the previous row */
        for (int i = 1; i < MAX_ROWS; i++) {
            memory_copy(VIDEO_ADDRESS + get_offset(0, i),
                        VIDEO_ADDRESS + get_offset(0, i-1),
                        MAX_COLS * 2);
        }

        /* Blank last line by setting all bytes to 0 */
        char* last_line =  VIDEO_ADDRESS + get_offset(0, MAX_ROWS-1);
        for (int i = 0; i < MAX_COLS * 2; i++) {
            last_line[i] = 0;
        }

        /* Move the offset back one row, such that it is now on the last
         * row, rather than off the edge of the screen */
        offset -= 2 * MAX_COLS;
    }
    return offset;
}


int get_offset(int col, int row) {
    return 2 * (row * MAX_COLS + col);
}
int get_offset_row(int offset) {
    return offset / (2 * MAX_COLS);
}
int get_offset_col(int offset) {
    return (offset - (get_offset_row(offset) * 2 * MAX_COLS)) / 2;
}