#include "preproc.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

using std::cout;
using std::endl;

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

// TODO: calculate numPartitions based on memoryBudget
void Preproc::setVIT(Context &c) {
	setNumEdges(c);	
	int numErules = c.grammar.getNumErules();
	int numPartitions = c.getNumPartitions();
	int total_size = totalNumEdges + (maxVid+1) * numErules;
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
	cout << "numVertex: " << maxVid+1 << endl;
	cout << "numEdges: " << total_size << endl;
	c.vit.print();
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
			labels[addr[i] + index[i]] = (char)(j-128);
			++index[i];
		}	
	}

	// sort edges of each vertex
	for(int i = 0;i <= maxVid;++i)
		quickSort3Way(vertices + addr[i],labels + addr[i],0,index[i]-1);
	
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
			if(!numEdges[j])
				continue;
			//remove duplicate edges
			vertexid_t *edge_v = new vertexid_t[edgeNum];
			char *edge_l = new char[edgeNum];
			*edge_v = vertices[addr[j]]; *edge_l = labels[addr[j]];
			int len = 1;
			for(int k = 1;k < edgeNum;++k) {
				if(vertices[addr[j] + k] == vertices[addr[j] + k-1] && labels[addr[j] + k] == labels[addr[j] + k-1]) {
					continue;
				}
				else {
					edge_v[len] = vertices[addr[j] + k];
					edge_l[len] = labels[addr[j] + k];
					++len;
				}
			}
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
	
	cout << "totalDuplicateEdges: " << totalDuplicateEdges << endl;

	delete[] addr;
	delete[] index;
	delete[] vertices;
	delete[] labels;
}

void insertSort(vertexid_t *A,char *B,int l,int r) {
	for(int j = l+1;j <= r;++j) {
		vertexid_t key_v = A[j];
		char key_c = B[j];
		int i = j-1;
		while(i >= l && key_v < A[i]) {
			A[i+1] = A[i];
			B[i+1] = B[i];
			--i;
		}
		A[i+1] = key_v;
		B[i+1] = key_c;
	}
}

void quickSort3Way(vertexid_t *A,char* B,int l,int r) {

	if(l >= r)
		return;	
	if(r - l + 1 <= 0)	
		insertSort(A,B,l,r);
	else {
		int pivot = getPivot(A,B,l,r);
		int p,q,i,j;
		i = p = l;
		j = q = r-1;

		while(1) {
			while(i < r && A[i] <= pivot) {
				if(A[i] == pivot) {
					std::swap(A[i],A[p]);
					std::swap(B[i],B[p]);
					++p;
				}
				++i;
			}
			while(l <= j && A[j] >= pivot) {
				if(A[j] == pivot) {
					std::swap(A[j],A[q]);
					std::swap(B[j],B[q]);
					--q;
				}	
				--j;
			}
			if(i >= j)
				break;
			std::swap(A[i],A[j]);
			std::swap(B[i],B[j]);
			++i;--j;
		}
		--i; --p;
		while(p >= l) {
			std::swap(A[i],A[p]);
			std::swap(B[i],B[p]);
			--i; --p;
		}
		++j; ++q;
		while(q <= r) {
			std::swap(A[j],A[q]);
			std::swap(B[j],B[q]);
			++j;++q;
		}

		quickSort3Way(A,B,l,i);
		quickSort3Way(A,B,j,r);
	}	
}

int getPivot(vertexid_t *A,char *B,int l,int r) {
	int mid = (l + r) / 2; int k = l;
	if(A[mid] < A[k]) k = mid;
	if(A[r] < A[k]) k = r;
	if(k != l) {
		std::swap(A[k],A[l]);
		std::swap(B[k],B[l]);
	}
	if(mid != r && A[mid] < A[r]) {
		std::swap(A[mid],A[r]);
		std::swap(B[mid],B[r]);
	}
	return A[r];
}

void Preproc::test(Context &c) {
	c.vit.print();
}
