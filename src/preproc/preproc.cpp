#include <fstream>
#include <cstdlib>
#include <string.h>

#include "preproc.h"
#include "../algorithm/myalgorithm.h"

Preproc::Preproc() {
	totalNumEdges = maxVid = totalDuplicateEdges = 0;	
}

Preproc::~Preproc() {
	if(numEdges != NULL)
		delete []numEdges;	
}

void Preproc::loadGrammar(Context &c) {
	c.grammar.loadGrammar(c.getGrammarFile());
}

void Preproc::setNumEdges(Context &c) {
	char *fname = c.getGraphFile();
	std::ifstream fin;
	fin.open(fname);
	if(!fin) {	
		cout << "can't load graph file: " << fname << endl;
		exit(-1);
	}

	char str[512];
	maxVid = totalNumEdges = 0;
	while(fin.getline(str,sizeof(str))) {
		char *p = strtok(str,"\t");
		if(p) {
			int value = atoi(p);
			if(value > maxVid) 
				maxVid = value;
		}	
	} 
	fin.close();

	numEdges = new vertexid_t[maxVid+1];
	for(int i = 0;i <= maxVid;++i)
			numEdges[i] = 0;
	FILE *fp = fopen(fname,"r");
	if(!fp) {
		cout << "can't load graph file: " << fname << endl;	
		exit(-1);
	}
	else {
		int src,dst; char rawLabel[GRAMMAR_STR_LEN];	
		while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
			++numEdges[src];
			++totalNumEdges;
		}	
	}
	fclose(fp);
	
}
/* two partition used memory: 8 * numEdges + 5 * numVertices
 * compset used memory: 0.55 * numEdges + 139 * numRealVertices (numRealVertices <= numVertices)
 * preproc need memory: 8.55 * numEdges + 5 * numVertices + 139 * numRealVertices;
 */	
void Preproc::setVIT(Context &c) {
	setNumEdges(c);	
	int numErules = c.grammar.getNumErules();
	int numPartitions = c.getNumPartitions();
	if(numPartitions <= 1) {
		numPartitions = 2;
		c.setNumPartitions(2);
	}
	int total_size = totalNumEdges + (maxVid+1) * numErules;
	int numRealVertices = 0;
	if(numErules) 
		numRealVertices = maxVid+1;
	else {
		for(int i = 0;i <= maxVid;++i) {
			if(numEdges[i])
				++numRealVertices;	
		}	
	}
	// numPartitions based on memBudget and user cmd.
	unsigned long int a = (unsigned long int)139 *numRealVertices + (unsigned long int)5 * (maxVid+1) + (unsigned long int)8.55 * total_size;
	unsigned long int b = c.getMemBudget() * 0.4;
	int minNumPartitions = a / b + 1;
	if(minNumPartitions > numPartitions) {
		numPartitions = (minNumPartitions <= 2) ? 2 : minNumPartitions;
		c.setNumPartitions(numPartitions);
	}

	int partition_size = (total_size) / numPartitions;
	int part_id = 0; int cur_size = 0; int i = 0; int v_start = 0;
	for(;i <= maxVid;++i) {
		cur_size += (numErules + numEdges[i]);
		if(part_id == numPartitions - 1) {
			for(int j = v_start+1;j <= maxVid;++j)
				cur_size += (numErules + numEdges[j]);
			c.vit.add(v_start,maxVid,cur_size);
			++part_id;
			break;
		}
		else {
			if(cur_size >= partition_size) {
				c.vit.add(v_start,i,cur_size);
				v_start = i+1;
				cur_size = 0;
				++part_id;
			}
		}	
	}
	cout << "NumVertex: " << maxVid+1 << endl;
	cout << "NumEdges: " << total_size << endl;
	c.ddm.setNumPartitions(numPartitions);
}

void Preproc::savePartitions(Context &c) {
	
	int numErules = c.grammar.getNumErules();	
	int size = totalNumEdges + (maxVid+1) * numErules;
	
	/* using 1D array instead of 2D array
	 * faster and smaller(RAM)
	 */
	int *addr = new int[(maxVid+1)];
	int address = 0;
	for(int i = 0;i <= maxVid;++i) {
		addr[i] = address;
		address += (numEdges[i] + numErules);
	}
	vertexid_t *vertices = new vertexid_t[size];
	char *labels = new char[size];
	int *index = new int[maxVid+1];
	for(int i = 0;i <= maxVid;++i)
		index[i] = 0;	
	
	// add edges of each vertex from graph_file
	FILE *fp = fopen(c.getGraphFile(),"r");
	if(!fp) {
		cout << "can't open graph_file: " << c.getGraphFile() << endl;
		exit(-1);
	}
	vertexid_t src,dst; char rawLabel[GRAMMAR_STR_LEN];
	while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
		vertices[addr[src] + index[src]] = dst;
		labels[addr[src] + index[src]] = c.grammar.getLabelValue(rawLabel);
		++index[src];
	}
	// add e-rule edges
	for(int i = 0;i <= maxVid;++i) {
		for(int j = 0;j < numErules;++j) {
			vertices[addr[i] + index[i]] = i;
			labels[addr[i] + index[i]] = c.grammar.getErule(j);
			++index[i];
		}	
	}

	// sort edges of each vertex
	for(int i = 0;i <= maxVid;++i)
		myalgo::quickSort(vertices + addr[i],labels + addr[i],0,index[i]-1);	
	
	/* remove duplicate edges of each vertex
	 * save partitions To file (binary format)
	 */
	for(int i = 0;i < c.vit.getSize();++i) {
		vertexid_t v_start,v_end;
		v_start = c.vit.getStart(i);
		v_end = c.vit.getEnd(i);
		int dupleNum = 0;

		char filename[256];
		sprintf(filename,"%d.part",i);	
		FILE *f = fopen(filename,"wb");
		for(vertexid_t j = v_start; j <= v_end;++j) {
			int edgeNum = numEdges[j] + numErules;
			if(!edgeNum)
				continue;
			
			//remove duplicate edges
			vertexid_t *edge_v = new vertexid_t[edgeNum];
			char *edge_l = new char[edgeNum];
			int len = 1;
			myalgo::removeDuple(len,edge_v,edge_l,edgeNum,vertices + addr[j],labels + addr[j]);
			dupleNum += (edgeNum - len);
			
			//save partitions to binary file
			fwrite((const void*)& j,sizeof(vertexid_t),1,f);
			fwrite((const void*)& len,sizeof(vertexid_t),1,f);
			for(int k = 0;k < len;++k) {
				fwrite((const void*)& edge_v[k],sizeof(vertexid_t),1,f);
				fwrite((const void*)& edge_l[k],sizeof(char),1,f);
			}
			delete[] edge_v; 
			delete[] edge_l;
		}
		fclose(f);	
		if(dupleNum) {
			totalDuplicateEdges += dupleNum;
			c.vit.setDegree(i,c.vit.getDegree(i)-dupleNum);
		}	
	}
	delete[] addr;
	delete[] index;
	delete[] vertices;
	delete[] labels;
	cout << "DUPLE EDGES: " << totalDuplicateEdges << endl;
}

void Preproc::test(Context &c) {
	c.vit.print();
}
