#include <pam.h>
#include <string.h>

struct pam inpam, outpam;
tuple *inrows[3];
tuple *outrow;

void
clear_inrow(struct pam *p, tuple *row)
{
	int i, j;
	
	for(i=0; i<p->width; i++)
		for(j=0; j<3; j++)
			row[i][j]=0;
}

void
clear_outrow(struct pam *p, tuple *row)
{
	int i,j;

	for(i=0; i<p->width; i++)
		for(j=0; j<3; j++)
			row[i][j]=p->maxval;
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
	if(is_black(1,column-1)) n++;
	if(is_black(1,column+1)) n++;
	return n==2 || n==3;
}

void
set_white(unsigned int column)
{
	sample *s = outrow[column];
	s[0]=255; s[1]=255; s[2]=255;
}

void
set_black(unsigned int column)
{
	sample *s = outrow[column];
	s[0]=0; s[1]=0; s[2]=0;
}

void
copy_tuple(unsigned int column)
{
	sample *src, *dest;

	src = inrows[1][column];
	dest = outrow[column];
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

void
read_row(unsigned int q)
{
	memcpy(inrows[0], inrows[1], outpam.width*sizeof(sample)*3);
	memcpy(inrows[1], inrows[2], outpam.width*sizeof(sample)*3);
	if(q) pnm_readpamrow(&inpam, inrows[2]+1);
	else clear_inrow(&outpam, inrows[2]);
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

	outrow = pnm_allocpamrow(&outpam);
	for(i=0; i<3; i++) inrows[i] = pnm_allocpamrow(&outpam);
	for(i=0; i<3; i++) clear_inrow(&outpam, inrows[i]);

	clear_outrow(&outpam, outrow);
	pnm_writepamrow(&outpam, inrows[0]);

	pnm_readpamrow(&inpam, inrows[1]+1);
	for(row=0; row < inpam.height; row++) {
		for(column=1; column<=inpam.width; column++) {
			if(is_black(1,column)) {
				if(is_edge(column)) set_black(column);
				else set_white(column);
			} else copy_tuple(column);
		}
		pnm_writepamrow(&outpam, outrow);
		read_row(row != inpam.height-1);
	}

	clear_outrow(&outpam, outrow);
	pnm_writepamrow(&outpam, inrows[2]);

	pnm_freepamrow(outrow);
	for(i=0; i<3; i++) pnm_freepamrow(inrows[i]);
	return 0;
}
