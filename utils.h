#pragma once
#include <stdio.h>

unsigned char* BmpToTexture(char*, int*, int*);
int				ReadInt(FILE*);
short			ReadShort(FILE*);

void			Cross(float[3], float[3], float[3]);
float			Dot(float[3], float[3]);
float			Unit(float[3], float[3]);
char* ReadRestOfLine(FILE*);
void	ReadObjVTN(char*, int*, int*, int*);
float	Unit(float[3]);
int
LoadObjFile(char* name);
void			Axes(float);
