#include <pam.h>
#include <string.h>

struct pam inpam, outpam;
tuple *inrows[3];
tuple *outrow;

void
row_fill_black(struct pam *p, tuple *row)
{
	int i, j;
	
	for(i=0; i<p->width; i++)
		for(j=0; j<3; j++)
			row[i][j]=0;
}

int
is_black(unsigned int row, unsigned int column)
{

	sample *s = inrows[row][column];
	if(s[0]==0 && s[1]==0 && s[2]==0) return ~0;
	else return 0;
}

int
is_edge(unsigned int column)
{
	unsigned int n = 0;
	if(is_black(0,column)) n++;
	if(is_black(2,column)) n++;
	if(column == 0 || is_black(1, column-1)) n++;
	if(column == outpam.width-1 || is_black(1, column+1)) n++;
	return n>1 && n<4;
}

void
set_black(unsigned int column)
{
	sample *s = outrow[column];
	s[0]=0; s[1]=0; s[2]=0;
}

void
set_white(unsigned int column)
{
	sample *s = outrow[column];
	s[0]=255; s[1]=255; s[2]=255;
}

void
copy_tuple(sample *dest, sample *src)
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

void
copy_row(struct pam *p, tuple *dest, tuple *src) 
{
	int i;

	for(i=0; i<p->width; i++) copy_tuple(dest[i], src[i]);
}

void
next_row(unsigned int read)
{
	copy_row(&outpam, inrows[0], inrows[1]);
	copy_row(&outpam, inrows[1], inrows[2]);
	/*
	memcpy(inrows[0], inrows[1], outpam.width*sizeof(sample)*3);
	memcpy(inrows[1], inrows[2], outpam.width*sizeof(sample)*3);
	*/
	if(read) pnm_readpamrow(&inpam, inrows[2]+1);
	else row_fill_black(&outpam, inrows[2]);
}


int
main(int argc, char **argv)
{
	unsigned int row,column,i;

	pnm_init(&argc, argv);
	pnm_readpaminit(stdin, &inpam, sizeof(struct pam));

	outpam = inpam;
	outpam.width+=2; outpam.height+=2;
	outpam.file = stdout;
	pnm_writepaminit(&outpam);

	for(i=0; i<3; i++) {
		inrows[i] = pnm_allocpamrow(&outpam);
		row_fill_black(&outpam, inrows[i]);
	}

	outrow = pnm_allocpamrow(&outpam);

	for(row=0; row < outpam.height; row++) {
		next_row(row < inpam.height);
		for(column=0; column<outpam.width; column++) {
			if(is_black(1,column)) {
				if(is_edge(column)) set_black(column);
				else set_white(column);
			} else copy_tuple(outrow[column], inrows[1][column]);
		}
		pnm_writepamrow(&outpam, outrow);
	}

	pnm_freepamrow(outrow);
	for(i=0; i<3; i++) pnm_freepamrow(inrows[i]);
	return 0;
}
