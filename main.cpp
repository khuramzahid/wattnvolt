// (Done) Objective 1: Traverse nodes from Child to Parent in VLR order
// (Done) Objective 2: Make the SLD of the Lay's factory as nodes
// (Done) Objective 3: Add Cable Data and CB Data as integral part of the nodes
// (Done) Objective 4: Perform Load Flow Analysis
// (Done) Objective 5: Perform Short Circuit Analysis
// Objective 6: Perform CB Coordination
// Objective 7: Perform Arc Flash Analysis
// Objective 8: Make all 5 PSA Scripts
// Objective 9: Repeat Steps 4-7 for all Remaining PSA Scripts


#include <iostream>
#include <string>
#include <fstream>
using namespace std;
string removeDash(string s) {
    for(int j=0;j<s.length();j++)
        if(s[j] == '-')
            s[j] = ' ';
    return s;
}
char* returnCharArray(string s) {
    char* ch = new char[s.length()+1];
    for(int j=0;j<s.length();j++)
        ch[j] = s[j];
    ch[s.length()] = '\0';
    return ch;
}
class node;

struct nodeRecord {
    node* nodeAddress;
    string description;
};

class nodeArray {
    public:
        nodeArray();
        ~nodeArray();
        void pushNode(node*);
        void editNode(node*);
        node* returnNode(string);
        nodeRecord* nr;
        int numRecord;
};

class node {

    public:
        node();
        ~node();
        void insertNode(node*, nodeArray&);
        void generateNodeReport(ofstream&);
        string inputData(ifstream&);
        void setChildVolts(float);
        void informCurrent(float);

        node* parentNode;
        node* childArray;
        int numChild;

        string description;
        string nodeType;
        float turnRatio;
        float imagSupply;

        float voltsMag;
        float cableVoltDrop;

        float resistance;

        float realLoad;

        float cableCurrent;
        float shortCircuitCurrent;

        string make;
        string model;
        float ir;
        float ics;
        float icu;

};

int main() {
    string command, type, description;
    cout<<"Open PSA Script: "<<endl;
    cout<<"1) Wapda: LT14Closed & LT25Open"<<endl;
    cout<<"2) Wapda: LT14Open & LT25Closed"<<endl;
    cout<<"3) Generator: LT1Closed & LT14Open"<<endl;
    cout<<"4) Generator: LT1Open & LT14Closed"<<endl;
    cout<<"5) Generator: LT1Closed & LT14Closed"<<endl;
    cout<<"6) TEST"<<endl;
    int fileIndex;
    ifstream i;
    cin>>fileIndex;
    if(fileIndex == 1) {
        i.open("W_14C_25O.psa");
        if(!i)
            cout<<"W_14C_25O.psa not found."<<endl;
    }
    else if(fileIndex == 2) {
        i.open("W_14O_25C.psa");
        if(!i)
            cout<<"W_14O_25C.psa not found"<<endl;
    }
    else if(fileIndex == 3) {
        i.open("G_1C_14O.psa");
        if(!i)
            cout<<"G_1C_14O.psa not found"<<endl;
    }
    else if(fileIndex == 4) {
        i.open("G_1O_14C.psa");
        if(!i)
            cout<<"G_1O_14C.psa not found"<<endl;
    }
    else if(fileIndex == 5) {
        i.open("G_1C_14C.psa");
        if(!i)
            cout<<"G_1C_14C.psa not found"<<endl;
    }
    else if(fileIndex == 6) {
        i.open("test.psa");
        if(!i)
            cout<<"test.psa not found"<<endl;
    }
    nodeArray na;
    int numStart = 0;
    node* startingNode = NULL; // Power is injected through these nodes
    node* thisNode = NULL;
    while(i) {
        i>>command;
        static int lineOfCode = 0;
        lineOfCode++;
        if(command == "declare") {
            i>>description;
            node* temp = startingNode;
            startingNode = new node[++numStart];
            for(int j=0; j<numStart-1; j++) {
                startingNode[j] = temp[j];
            }
            startingNode[numStart-1].description = description;
            i>>description; // volts string
            i>>startingNode[numStart-1].voltsMag;
            for(int j=0; j<numStart; j++) {
                if(na.returnNode(startingNode[j].description) == NULL)
                    na.pushNode(&startingNode[j]);
                else
                    na.editNode(&startingNode[j]);
            }
        }
        else if(command == "insert") {
            i>>type>>description;
            string stat;
            if(na.returnNode(description) == NULL) {
                node *n = new node();
                n->nodeType = type;
                n->description = description;
                stat = n->inputData(i);
                if(n->nodeType == "CB") {
                    if(stat == "OK")
                        thisNode->insertNode(n, na);
                    else if(stat == "Fault 1") {
                        cout<<description<< "is not in the file 'CB_Description.txt' (Line #: "<<lineOfCode<<")."<<endl;
                        break;
                    }
                    else {
                        cout<<stat<<" is not in the file 'CB_Detailed_description.txt' (Line #: "<<lineOfCode<<")."<<endl;
                        break;
                    }
                }
                else
                    thisNode->insertNode(n, na);
            }
            else {
                cout<<description<<" is already registered as a node (Line #: "<<lineOfCode<<")."<<endl;
                break;
            }
        }
        else if(command == "goto") {
            i>>description;
            thisNode = na.returnNode(description);
            if(thisNode == NULL) {
                cout<<description<<" is not recognized as a registered node (Line #: "<<lineOfCode<<")."<<endl;
                break;
            }
        }
        else if(command == "break") {
            i.close();
            thisNode = NULL;
            cout<<"*.psa has been read."<<endl;
            cout<<"Successful execution!"<<endl;
            break;
        }
        else {
            cout<<"Invalid Command: "<<command<<" is not recognized! (Line #: "<<lineOfCode<<")."<<endl;
            break;
        }
    }
    // ------------------------------------------------------LOAD FLOW -------------------------------------------------
    for(int repeat = 0; repeat < 500; repeat++) {
        // -------------- Set Volts on all Buses -----------------
        for(int j=0;j<numStart;j++) {
            startingNode[j].setChildVolts(startingNode[j].voltsMag);
        }
        // --------------- Compute Cable Currents -----------------
        for(int j=0;j<na.numRecord;j++) {
            thisNode = na.nr[j].nodeAddress;
            thisNode->cableCurrent = 0;
        }
        for(int j=0;j<na.numRecord;j++) {
            thisNode = na.nr[j].nodeAddress;
            if(thisNode->realLoad != 0) {
                thisNode->cableCurrent = thisNode->realLoad / (thisNode->voltsMag * 0.8 * 1.732);
                thisNode->informCurrent(thisNode->cableCurrent);
            }
        }
        // --------------- Compute Cable Voltages ----------------
        for(int j=0;j<na.numRecord;j++) {
            thisNode = na.nr[j].nodeAddress;
            thisNode->cableVoltDrop = thisNode->resistance * thisNode->cableCurrent;
        }
    }
    // ------------------------------------------------------- SHORT CIRCUIT ------------------------------------------
    // -------------- Compute Short Circuit Current ----------------
    for(int j=0;j<na.numRecord;j++) {
        thisNode = na.nr[j].nodeAddress;
        if(thisNode->parentNode != NULL && thisNode->resistance != 0)
            thisNode->shortCircuitCurrent = 0.001 * 1.732 * thisNode->parentNode->voltsMag/thisNode->resistance;
    }
    // -------------------------------------------------------- PSA REPORT ---------------------------------------
    ofstream o;
    o.open("Power System Analysis Report.xlsx");
    o<<"DESCRIPTION"<<"\t";
    o<<"VOLTAGE (volts)"<<"\t";
    o<<"TYPE"<<"\t";
    o<<"POWER RATING (kW)"<<"\t";
    o<<"TURNRATIO"<<"\t";
    o<<"MAKE/MODEL"<<"\t";
    //o<<"MODEL"<<"\t";
    o<<"CURRENT COMSUMPTION (A)"<<"\t";
    o<<"RATED CURRENT (A)"<<"\t";
    o<<"SHORT CIRCUIT - Ics (kA)"<<"\t";
    o<<"SHORT CIRCUIT - Icu (kA)"<<"\t";
    o<<"CALCULATED SHORT CIRCUIT CURRENT (kA)"<<"\t";
    o<<"SUPPLY FROM"<<"\t";
    o<<"COORDINATION STATUS"<<"\t";
    for(int j=0;j<na.numRecord;j++) {
        na.nr[j].nodeAddress->generateNodeReport(o);
        o<<endl;
    }
    cout<<"Node Reports have been generated/updated."<<endl;
    cout<<"Number of Nodes: "<<na.numRecord<<endl;
    return 0;
}

// -------------------------------- NODE ----------------------------------
node::node() {
    childArray = NULL;
    parentNode = NULL;
    numChild = 0;
    description = "default";
    voltsMag = 0;
    resistance = 0;
    nodeType = "-";
    realLoad = 0;
    turnRatio = 1;
    imagSupply = 0;
    cableCurrent = 0;
    cableVoltDrop = 0;
    shortCircuitCurrent = 0;
    make = "-";
    model = "-";
    ir = 0;
    ics = 0;
    icu = 0;
}
node::~node() {
    delete[] childArray;
    delete parentNode;
}
void node::insertNode(node *n, nodeArray& na) {
    node* temp = childArray;
    childArray = new node[++numChild];
    for(int j=0; j<numChild-1; j++) {
        childArray[j] = temp[j];
    }
    childArray[numChild-1] = *n;
    childArray[numChild-1].parentNode = this;
    for(int j=0; j<numChild; j++) {
        if(na.returnNode(childArray[j].description) == NULL)
            na.pushNode(&childArray[j]);
        else
            na.editNode(&childArray[j]);
    }
}
string node::inputData(ifstream &i) {
    string cbString, returnString = "OK";
	int cableSets;
    i>>cbString;
    i>>cableSets;
    int condPerPhase;
    float area;
    string cores;
    float length;
    for(int j=0; j<cableSets; j++) {
        i>>condPerPhase;
        i>>area;
        i>>cores;
        i>>length;
		float newResistance;
		if(cores == "4c" || cores == "3c" || cores == "3.5c" || cores == "4c/BUS")
			newResistance = (1.68*length*condPerPhase)/(area*100); // single conductor resistance
		else if (cores == "1c/XLPE")
			newResistance = (2.65*length*condPerPhase)/(area*100); // single conductor resistance
		if(cableSets != 1 && j != 0)
			resistance = (resistance*newResistance)/(resistance+newResistance);
		else
			resistance = newResistance;
    }
    if(nodeType == "LOAD") {
        i>>cbString; // load string
        i>>realLoad;
    }
    else if(nodeType == "CB") {
        ifstream i1;
        i1.open("CB_Description.txt");
        while(i1) {
            i1>>cbString;
            if(cbString == description) {
                i1>>make;
                i1>>model;
                i1>>ir;
                break;
            }
        }
        i1.close();
        if(cbString != description)
            returnString = "Fault 1";
        string cbString2;
        ifstream i2;
        i2.open("CB_Detailed_Description.txt");
        while(i2) {
            i2>>cbString>>cbString2;
            if(cbString == make && cbString2 == model) {
                i2>>ics;
                i2>>icu;
                break;
            }
        }
        i2.close();
        if(cbString != make && cbString2 != model)
            returnString = make+" "+model;
    }
    else if(nodeType == "PFI") {
        i>>cbString; // supply string
        i>>imagSupply;
    }
    else if(nodeType == "TR") {
        i>>cbString; // turnRatio string
        i>>voltsMag>>turnRatio;
        turnRatio = voltsMag/turnRatio;
        voltsMag = 0;
    }
    return returnString;
}
void node::generateNodeReport(ofstream &o) {
    o<<removeDash(description)<<"\t";
    o<<voltsMag<<"\t";
    o<<nodeType<<"\t";
    o<<realLoad<<"\t";
    o<<turnRatio<<"\t";
    o<<make<<" ";
    o<<model<<"\t";
    o<<cableCurrent<<"\t";
    o<<ir<<"\t";
    o<<ics<<"\t";
    o<<icu<<"\t";
    o<<shortCircuitCurrent<<"\t";
    if(parentNode != NULL)
        o<<removeDash(parentNode->description)<<"\t";
    o<<"Not Applicable"<<"\t";
}
void node::setChildVolts(float voltValue) {
    for(int j=0;j<numChild;j++) {
        childArray[j].voltsMag = (voltValue/turnRatio) - childArray[j].cableVoltDrop;
        childArray[j].setChildVolts(childArray[j].voltsMag);
    }
}
void node::informCurrent(float currentValue) {
    if(parentNode != NULL) {
        parentNode->cableCurrent += currentValue/turnRatio;
        parentNode->informCurrent(currentValue/turnRatio);
    }
}
// --------------------------------- NODE ARRAY --------------------------------
nodeArray::nodeArray() {
    nr = NULL;
    numRecord = 0;
}
nodeArray::~nodeArray() {
    delete[] nr;
}
void nodeArray::pushNode(node* n) {
    nodeRecord* temp = nr;
    nr = new nodeRecord[++numRecord];
    for(int j=0; j<numRecord-1; j++)
        nr[j] = temp[j];
    nr[numRecord-1].nodeAddress = n;
    nr[numRecord-1].description = n->description;
}
void nodeArray::editNode(node* n) {
    for(int j=0; j<numRecord; j++)
        if(nr[j].description == n->description)
            nr[j].nodeAddress = n;
}
node* nodeArray::returnNode(string s) {
    for(int j=0; j<numRecord; j++)
        if(nr[j].description == s)
            return nr[j].nodeAddress;
    return NULL;
}
