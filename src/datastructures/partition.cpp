#include <stdio.h>
#include <cstdlib>
#include "partition.h"
#include "../algorithm/myalgorithm.h"

Partition::Partition() {
	id = -1;
	numVertices = numEdges = 0;
}

Partition::Partition(partitionid_t id) {
	this->id = id;
	numVertices = numEdges = 0;
}

void Partition::clear() {
	if(id != -1 && numVertices && numEdges) {	
		if(vertices) delete[] vertices;
		if(labels) delete[] labels;
		if(addr) delete[] addr;
		if(index) delete[] index;
		vertices = NULL; labels = NULL; addr = NULL; index = NULL;
		id = -1; numVertices = numEdges = 0;
	}	
}

bool Partition::check() {
	int curIndex = 0;	
	for(int i = 0;i < numVertices;++i) {
		if(myalgo::checkEdges(index[i],vertices + addr[i],labels + addr[i]))
			return true;	
	}	
	return false;	
}

void Partition::loadFromFile(partitionid_t id,Context &c) {
	char fname[256];
	sprintf(fname,"%d.part",id);
	FILE *fp = fopen(fname,"rb");
	if(!fp) {
		cout << "can't load partition file: " << fname << endl;
		exit(-1);
	}
	this->id = id;
	vertexid_t v_start = c.vit.getStart(id);
	vertexid_t v_end = c.vit.getEnd(id);
	numVertices = v_end - v_start + 1;
	numEdges = c.vit.getDegree(id);
	vertexid_t src,dst,degree;
	char label;
	
	vertices = new vertexid_t[numEdges];
	labels = new char[numEdges];
	addr = new vertexid_t[numVertices];
	index = new vertexid_t[numVertices];
	for(int i = 0;i < numVertices;++i) {
		addr[i] = index[i] = 0;
	}
	int cur_addr = 0;

	while(fread(&src,sizeof(vertexid_t),1,fp) != 0) {
		fread(&degree,sizeof(vertexid_t),1,fp);
		vertexid_t vid = src - v_start;
		addr[vid] = cur_addr;
		int bufsize = (sizeof(vertexid_t) + sizeof(char)) * degree;	
		char *buf = (char*)malloc(bufsize);
		fread(buf,bufsize,1,fp);
		// sizeof(vertexid_t) + sizeof(char) = 5
		for(vertexid_t i = 0;i < bufsize;i += 5) {
			dst = *((vertexid_t*)(buf + i));
			label = *((char*)(buf + 4 + i));
			vertices[cur_addr + index[vid]] = dst;
			labels[cur_addr + index[vid]] = label;
			++index[vid];
		}
		free(buf);
		cur_addr += degree;
	}
	fclose(fp);
}

void Partition::print(Context &c) {
	cout << "========Partition test begin======" << endl;
	cout << "partition id: " << id << endl;
	cout << "numVertices: " << numVertices << endl;
	vertexid_t v_start = c.vit.getStart(id);
	for(vertexid_t i = 0;i < numVertices;++i) {
		if(index[i]) {
			cout << "(" << v_start + i << "," << index[i] << ")" << endl;	
			for(int j = 0;j < index[i];++j) {
				cout << "(" << vertices[addr[i]+j] << "," << (int)labels[addr[i]+j]	 << ") "<< "-> ";
			}		
			cout << "end" << endl;
		}	
		else {
			cout << "empty edgelist" << endl;	
		}
	}	
}
	
