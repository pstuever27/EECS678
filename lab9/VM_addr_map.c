#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


#define MAXSTR 1000

int main(int argc, char *argv[])
{
  char line[MAXSTR];
  int *page_table, *mem_map;
  unsigned int log_size, phy_size, page_size, d;
  unsigned int num_pages, num_frames;
  unsigned int offset, logical_addr, physical_addr, page_num, frame_num;

  /* Get the memory characteristics from the input file */
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Logical address space size: %d^%d", &d, &log_size)) != 2){
    fprintf(stderr, "Unexpected line 1. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Physical address space size: %d^%d", &d, &phy_size)) != 2){
    fprintf(stderr, "Unexpected line 2. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Page size: %d^%d", &d, &page_size)) != 2){
    fprintf(stderr, "Unexpected line 3. Abort.\n");
    exit(-1);
  }

  /* Allocate arrays to hold the page table and memory frames map */
  num_frames = (int) pow( 2, phy_size - page_size );
  num_pages = (int) pow( 2, log_size - page_size );
  printf("Number of Pages: %d, Number of Frames: %d\n", num_pages, num_frames );

  page_table = (int *) malloc(num_pages * sizeof(int));
  mem_map = (int *) malloc(num_frames * sizeof(int));

  /* Initialize page table to indicate that no pages are currently mapped to
     physical memory */

  memset( page_table, 0, sizeof(page_table));
  memset( mem_map, 0, sizeof(mem_map));

  int temp = 0;
  int frame = 0;
  printf("\n");
  

  /* Initialize memory map table to indicate no valid frames */
  

  /* Read each accessed address from input file. Map the logical address to
     corresponding physical address */
  fgets(line, MAXSTR, stdin);
  while(!(feof(stdin))){
    sscanf(line, "0x%x", &logical_addr);
    fprintf(stdout, "Logical Address: 0x%x\n", logical_addr);
    
	/* Calculate page numbber and offset from the logical address */
  offset = logical_addr & (0xFFFFFFFF >> ( log_size - page_size ));
  page_num = (logical_addr & ( 0xFFFFFFFF << page_size )) >> page_size;

  printf("Page Number: %d\n", page_num);

    /* Form corresponding physical address */
  if( page_table[page_num] == 0 )
  {
    printf( "Page Fault!\n" );
    printf( "Frame Number: %d\n", temp );

    mem_map[temp] = offset;
    physical_addr = ( temp << page_size ) | offset;
    page_table[page_num] = temp + 1;
    printf( "Physical Address: 0x%x\n", physical_addr );
    temp++;
  }
  else
  {
    frame = page_table[page_num];
    frame--;
    printf("Frame Number: %d\n", frame );
    physical_addr = (frame << page_size ) | offset;
    printf("Physical Address: 0x%x\n", physical_addr );
  }
    
    /* Read next line */
    printf("\n");
    fgets(line, MAXSTR, stdin);    
  }

  return 0;
}
