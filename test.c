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
#include "MyLibs/AndysOpenCvLib.h"
#include "MyLibs/version.h"



Protocol* CreateTestProtocol(char* name){

	Protocol* myP=CreateProtocolObject();
	myP->Description="A test protocol.";
	myP->Filename=name;

	myP->GridSize=cvSize(201,99);
	/** Create the Steps Object and Load it into the Protocol **/
	myP->Steps=CreateStepsObject(myP->memory);


	/** Create Some Polygons **/
	WormPolygon* Head = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Left = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Right = CreateWormPolygon(myP->memory,myP->GridSize);
	WormPolygon* Tail = CreateWormPolygon(myP->memory,myP->GridSize);

	//(length along centerline,radius from centerline)
	cvSeqPush(Head->Points,&cvPoint(-100,0));
	cvSeqPush(Head->Points,&cvPoint(100,0));
	cvSeqPush(Head->Points,&cvPoint(100,20));
	cvSeqPush(Head->Points,&cvPoint(-100,20));

	cvSeqPush(Left->Points,&cvPoint(-100,30));
	cvSeqPush(Left->Points,&cvPoint(0,30));
	cvSeqPush(Left->Points,&cvPoint(0,70));
	cvSeqPush(Left->Points,&cvPoint(-100,70));

	cvSeqPush(Right->Points,&cvPoint(0,30));
	cvSeqPush(Right->Points,&cvPoint(100,30));
	cvSeqPush(Right->Points,&cvPoint(100,70));
	cvSeqPush(Right->Points,&cvPoint(0,70));

	cvSeqPush(Tail->Points,&cvPoint(-100,80));
	cvSeqPush(Tail->Points,&cvPoint(0,80));
	cvSeqPush(Tail->Points,&cvPoint(0,99));
	cvSeqPush(Tail->Points,&cvPoint(-100,99));


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

	printf("Head->points->total=%d\n",Head->Points->total);
	DestroyWormPolygon(&Head);
	DestroyWormPolygon(&Tail);
	DestroyWormPolygon(&Left);
	DestroyWormPolygon(&Right);
	return myP;

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
	printf("myP->Description=%s\n",myP->Description);

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



	/** Create a local object that contains the information of the steps **/
	CvSeq* stepSeq=node->data.seq;
	int numsteps=stepSeq->total;
	printf("numsteps=%d\n",numsteps);

	CvSeqReader StepReader;
	cvStartReadSeq( stepSeq, &StepReader, 0 );

	/** Let's loop through all of the steps **/
	for (int i= 0; i< numsteps; ++i) {

		/**Create Illumination Montage Object **/
		CvSeq* montage=CreateIlluminationMontage(myP->memory);

		/** Find the node of the current image montage (step) **/
		CvFileNode* montageNode = (CvFileNode*)StepReader.ptr;

		CvSeq* montageSeq=montageNode->data.seq;
		int numPolygonsInMontage=montageSeq->total;
		printf("Step %d: %d polygon(s) found\n",i,numPolygonsInMontage);

		CvSeqReader MontageReader;
		cvStartReadSeq( montageSeq, &MontageReader, 0 );

		/** Loop through all of the polygons **/
		for (int k = 0; k < numPolygonsInMontage; ++k) {
			/** Load the CvSeq Polygon Objects and push them onto the montage **/
			CvFileNode* polygonNode = (CvFileNode*)MontageReader.ptr;
			CvSeq* polygonPts =(CvSeq*) cvRead(fs,polygonNode); // <---- Andy come back here.
			printf("\tPolygon %d: found %d points.\n",k,polygonPts->total);

			/**
			 * Now we have the points for our polygon so we need to load
			 * those points into a polygon object
			 */
			WormPolygon* polygon= CreateWormPolygonFromSeq(myP->memory,myP->GridSize,polygonPts);
			printf("\t\t %d points copied\n",polygon->Points->total);

			/** Add the polygon to the montage **/
			cvSeqPush(montage,&polygon);
			printf("\t\t Current montage now has %d polygons\n",montage->total);

			/** Move to the next polygon **/
			CV_NEXT_SEQ_ELEM( montageSeq->elem_size, MontageReader );
		}
		cvClearSeq(montageSeq);
		numPolygonsInMontage=0;

		printf("Loading a montage with %d polygons on the protocol\n.",montage->total);
		/** Load the montage onto the step object**/
		cvSeqPush(myP->Steps,&montage);


		/** Progress to the next step **/
		CV_NEXT_SEQ_ELEM( stepSeq->elem_size, StepReader );

	}

	return myP;


}


int main(){

	//char* name = (char*) malloc(sizeof(char)*50);



	printf(copyString("Hello you World\n"));
	Protocol* myP=CreateTestProtocol("protocol.yml");
	VerifyProtocol(myP);

	WriteProtocolToYAML(myP);
	DestroyProtocolObject(&myP);

	printf("Done writing...\n\n");
	Protocol* protocol2 = ReadTestProtocol("protocol.yml");
	VerifyProtocol(protocol2);
	protocol2->Filename="protoco2.yml";

	printf("protocol2->Steps->total=%d\n",protocol2->Steps->total);
	WriteProtocolToYAML(protocol2);


	printf("Points between line test\n");
	cvWaitKey(0);
	CvMemStorage* mem= cvCreateMemStorage();
	CvSeq* test=cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint),mem);
	GetLineFromEndPts(cvPoint(0,0),cvPoint(10,15),test);



	printf("GenerateRectangleWorm\n");



	IplImage* rectWorm= GenerateRectangleWorm(protocol2->GridSize);
	printf("ShowImage\n");
	cvNamedWindow("RectWorm");
	cvShowImage("RectWorm",rectWorm);
	int k;
	for (k = 0; k < protocol2->Steps->total; ++k) {
		printf("====Step Number %d====\n",k);
		cvZero(rectWorm);
		IllumRectWorm(rectWorm,protocol2,k);
		cvShowImage("RectWorm",rectWorm);
		cvWaitKey(0);

	}


	return 0;
}


