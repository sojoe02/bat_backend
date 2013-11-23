#include<stdio.h>

#include"recorder.h"

Recorder::Recorder()
	:value(50)
{

}


void Recorder::printValue(){
	printf("Value is %i\n",value);
}

