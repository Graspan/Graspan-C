#include <fstream>
#include <cstdlib>
#include <string.h>
#include <limits.h>

#include "preproc.h"
#include "../algorithm/myalgorithm.h"

Preproc::Preproc() {
	totalNumEdges = totalNumVertices = maxVid = totalDuplicateEdges = 0;	
	minVid = INT_MAX;
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
			if(value < minVid)
				minVid = value;	
		}	
	} 
	fin.close();
	totalNumVertices = maxVid-minVid+1;

	numEdges = new vertexid_t[totalNumVertices];
	for(vertexid_t i = 0;i < totalNumVertices;++i)
			numEdges[i] = 0;
	FILE *fp = fopen(fname,"r");
	if(!fp) {
		cout << "can't load graph file: " << fname << endl;	
		exit(-1);
	}
	else {
		vertexid_t src,dst; char rawLabel[GRAMMAR_STR_LEN];	
		while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
			++numEdges[src-minVid];
			++totalNumEdges;
		}	
	}
	fclose(fp);
	
}
/* two partition used memory: 5 * numEdges + 12 * numVertices
 * compset used memory: 0.55 * numEdges + 139 * numRealVertices (numRealVertices <= numVertices)
 * preproc need memory: 5.55 * numEdges + 12 * numVertices + 139 * numRealVertices;
 */	
void Preproc::setVIT(Context &c) {
	setNumEdges(c);	
	int numErules = c.grammar.getNumErules();
	int numPartitions = c.getNumPartitions();
	if(numPartitions <= 1) {
		numPartitions = 2;
		c.setNumPartitions(numPartitions);
	}
	long total_size = totalNumEdges + totalNumVertices * numErules;
	vertexid_t numRealVertices = 0;
	if(numErules) 
		numRealVertices = totalNumVertices;
	else {
		for(int i = 0;i < totalNumVertices;++i) {
			if(numEdges[i])
				++numRealVertices;	
		}	
	}
	// numPartitions based on memBudget and user cmd.
	unsigned long int a = (unsigned long int)139 *numRealVertices + (unsigned long int)12 * totalNumVertices + 5.55 * (unsigned long int)total_size;
	unsigned long int b = c.getMemBudget() * 0.4;
	int minNumPartitions = a / b + 1;
	if(minNumPartitions > numPartitions) {
		numPartitions = (minNumPartitions <= 2) ? 2 : minNumPartitions;
		c.setNumPartitions(numPartitions);
	}

	long partition_size = (total_size) / numPartitions;
	partitionid_t part_id = 0; long cur_size = 0; vertexid_t v_start = minVid;
	for(vertexid_t i = 0;i < totalNumVertices;++i) {
		cur_size += (numErules + numEdges[i]);
		if(part_id == numPartitions - 1) {
			for(vertexid_t j = v_start+1-minVid;j < totalNumVertices;++j)
				cur_size += (numErules + numEdges[j]);
			c.vit.add(v_start,maxVid,cur_size);
			++part_id;
			break;
		}
		else {
			if(cur_size >= partition_size) {
				c.vit.add(v_start,i+minVid,cur_size);
				v_start = i+1+minVid;
				cur_size = 0;
				++part_id;
			}
		}	
	}
	cout << "NumVertex: " << totalNumVertices << endl;
	cout << "NumEdges: " << total_size << endl;
	c.ddm.setNumPartitions(numPartitions);
}

void Preproc::savePartitions(Context &c) {
	
	int numErules = c.grammar.getNumErules();	
	long size = totalNumEdges + totalNumVertices * numErules;
	
	/* using 1D array instead of 2D array
	 * faster and smaller(RAM)
	 */
	long *addr = new long[totalNumVertices];
	long address = 0;
	for(vertexid_t i = 0;i < totalNumVertices;++i) {
		addr[i] = address;
		address += (numEdges[i] + numErules);
	}
	vertexid_t *vertices = new vertexid_t[size];
	char *labels = new char[size];
	int *index = new int[totalNumVertices];
	for(vertexid_t i = 0;i < totalNumVertices;++i)
		index[i] = 0;	
	
	// add edges of each vertex from graph_file
	FILE *fp = fopen(c.getGraphFile(),"r");
	if(!fp) {
		cout << "can't open graph_file: " << c.getGraphFile() << endl;
		exit(-1);
	}
	vertexid_t src,dst; char rawLabel[GRAMMAR_STR_LEN];
	while(fscanf(fp,"%d\t%d\t%s\n",&src,&dst,rawLabel) != EOF) {
		vertices[addr[src-minVid] + index[src-minVid]] = dst;
		labels[addr[src-minVid] + index[src-minVid]] = c.grammar.getLabelValue(rawLabel);
		++index[src-minVid];
	}

	// add e-rule edges
	for(int i = 0;i < totalNumVertices;++i) {
		for(int j = 0;j < numErules;++j) {
			vertices[addr[i] + index[i]] = i + minVid;
			labels[addr[i] + index[i]] = c.grammar.getErule(j);
			++index[i];
		}	
	}

	// sort edges of each vertex
	for(int i = 0;i < totalNumVertices;++i)
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
			int edgeNum = numEdges[j-minVid] + numErules;
			if(!edgeNum)
				continue;
			
			//remove duplicate edges
			vertexid_t *edge_v = new vertexid_t[edgeNum];
			char *edge_l = new char[edgeNum];
			int len = 1;
			myalgo::removeDuple(len,edge_v,edge_l,edgeNum,vertices + addr[j-minVid],labels + addr[j-minVid]);
			dupleNum += (edgeNum - len);
			
			//initial DDM matrix
			for(int k = 0;k < len;++k)
				c.ddm.addDDM(i,c.vit.getPartitionId(edge_v[k]));	

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
	delete[] addr; delete[] index; delete[] vertices; delete[] labels;
	cout << "DUPLE EDGES: " << totalDuplicateEdges << endl;
}

void Preproc::test(Context &c) {
	c.vit.print();
}
