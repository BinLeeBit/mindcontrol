//Standard C headers
#include <stdio.h>
#include <ctime>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <string.h>


//C++ header
#include <iostream>
#include <limits>

using namespace std;

//OpenCV Headers
#include <highgui.h>
#include <cv.h>
#include <cxcore.h>



//Andy's Headers
#include "MyLibs/IllumWormProtocol.h"
#include "MyLibs/version.h"



void WriteTestProtocol(char* name){

	Protocol* myP=CreateProtocolObject();
	myP->Description="A test protocol.";
	myP->Filename=name;

	myP->GridSize=cvSize(2,99);
	/** Create the Steps Object and Load it into the Protocol **/
	myP->Steps=CreateStepsObject(myP->memory);


	/** Create Some Polygons **/
	WormPolygon* Head = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Left = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Right = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Tail = CreateWormPolygon(myP->memory,myP->GridSize);

	//(length along centerline,radius from centerline)
	cvSeqPush(Head->Points,&cvPoint(0,0));
	cvSeqPush(Head->Points,&cvPoint(2,0));
	cvSeqPush(Head->Points,&cvPoint(2,20));
	cvSeqPush(Head->Points,&cvPoint(0,20));

	cvSeqPush(Left->Points,&cvPoint(0,30));
	cvSeqPush(Left->Points,&cvPoint(1,30));
	cvSeqPush(Left->Points,&cvPoint(1,70));
	cvSeqPush(Left->Points,&cvPoint(0,70));

	cvSeqPush(Right->Points,&cvPoint(1,30));
	cvSeqPush(Right->Points,&cvPoint(2,30));
	cvSeqPush(Right->Points,&cvPoint(2,70));
	cvSeqPush(Right->Points,&cvPoint(1,70));

	cvSeqPush(Tail->Points,&cvPoint(0,80));
	cvSeqPush(Tail->Points,&cvPoint(1,80));
	cvSeqPush(Tail->Points,&cvPoint(1,99));
	cvSeqPush(Tail->Points,&cvPoint(0,99));


	/** Create an Illumination Montage**/
	CvSeq* FirstIllum=CreateIlluminationMontage(myP->memory);
	CvSeq* SecondIllum=CreateIlluminationMontage(myP->memory);
	CvSeq* ThirdIllum=CreateIlluminationMontage(myP->memory);


	/** Let's load up the illumination montages with polygons**/
	cvSeqPush(FirstIllum,&Head);
	cvSeqPush(FirstIllum,&Tail);
	cvSeqPush(FirstIllum,&Left);
	cvSeqPush(FirstIllum,&Right);

	printf("FirstIllum->total=%d\n",FirstIllum->total);

	cvSeqPush(SecondIllum,&Head);
	cvSeqPush(SecondIllum,&Right);


	cvSeqPush(ThirdIllum,&Tail);
	cvSeqPush(ThirdIllum,&Left);



	/** Let's Load the montages into a series of steps **/
	cvSeqPush(myP->Steps,&FirstIllum);
	cvSeqPush(myP->Steps,&SecondIllum);
	cvSeqPush(myP->Steps,&ThirdIllum);


	printf("Writing test protocol in file: %s\n",myP->Filename);
	WriteProtocolToYAML(myP);

	DestroyWormPolygon(&Head);
	DestroyWormPolygon(&Tail);
	DestroyWormPolygon(&Left);
	DestroyWormPolygon(&Right);
	DestroyProtocolObject(&myP);

}





Protocol* ReadTestProtocol(const char* name){
	printf("In ReadTestProtocol()");
	Protocol* myP=CreateProtocolObject();
	LoadProtocolWithFilename(name,myP);
	CvFileStorage* fs=cvOpenFileStorage("protocol.yml",0,CV_STORAGE_READ);
	printf("Opened File Storage\n");

	/** Point to Protocol Object **/
	CvFileNode* protonode=cvGetFileNodeByName(fs,NULL,"Protocol");


	/** Load in Description **/
	CvFileNode* node=cvGetFileNodeByName(fs,protonode,"Description");
	myP->Description=copyString(cvReadString(node,NULL));
	printf("myP->Description=%s",myP->Description);

	/** Load in Grid Size **/
	node=cvGetFileNodeByName(fs,protonode,"GridSize");
	int height=cvReadIntByName(fs,node,"height",-1);
	int width=cvReadIntByName(fs,node,"width",-1);
	if (height>0 && width>0){
		myP->GridSize=cvSize(width,height);
	}

	/** Create the Steps Object and Load it into the Protocol **/
	myP->Steps=CreateStepsObject(myP->memory);

	/** Point to the Steps node  in the YAML file **/
	node=cvGetFileNodeByName(fs,protonode,"Steps");

	return myP;


}


int main(){

	//char* name = (char*) malloc(sizeof(char)*50);



	printf(copyString("Hello you World\n"));

	WriteTestProtocol("protocol.yml");

	printf("Done writing...\n\n");
	Protocol* myP = ReadTestProtocol("protocol.yml");


	return 0;
}


