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
	for(int i = 0;i < numVertices;++i) {
			if(myalgo::checkEdges(index[i],vertices + addr[i],labels + addr[i])) 
				return true;	
	}	
	return false;	
}

void Partition::writeToFile(partitionid_t id,Context &c) {
	char filename[256];
	sprintf(filename,"%d.part",id);
	FILE *f = fopen(filename,"wb");
	vertexid_t v_start,v_end;
	v_start = c.vit.getStart(id);
	v_end = c.vit.getEnd(id);

	for(vertexid_t i = v_start;i <= v_end;++i) {
		int pos = i-v_start;	
		if(index[pos] == 0)
			continue;
		fwrite((const void*)& i,sizeof(vertexid_t),1,f);
		fwrite((const void*)& index[pos],sizeof(vertexid_t),1,f);
		for(int j = 0;j < index[pos];++j) {
			fwrite((const void*)& vertices[addr[pos] + j],sizeof(vertexid_t),1,f);
			fwrite((const void*)& labels[addr[pos] + j],sizeof(char),1,f);
		}
	}
	fclose(f);
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

	size_t freadRes = 0; // clear warnings
	while(fread(&src,sizeof(vertexid_t),1,fp) != 0) {
		freadRes = fread(&degree,sizeof(vertexid_t),1,fp);
		vertexid_t vid = src - v_start;
		addr[vid] = cur_addr;
		int bufsize = (sizeof(vertexid_t) + sizeof(char)) * degree;	
		char *buf = (char*)malloc(bufsize);
		freadRes = fread(buf,bufsize,1,fp);
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

void Partition::update(vertexid_t numVertices,vertexid_t numEdges,vertexid_t *vertices,char *labels,vertexid_t *addr,vertexid_t *index) {
	partitionid_t newId = this->id;
	this->clear();
	this->id = newId; this->numEdges = numEdges; this->numVertices = numVertices;
	this->vertices = new vertexid_t[numEdges]; this->labels = new char[numEdges];
	this->addr = new vertexid_t[numVertices]; this->index = new vertexid_t[numVertices];
	memcpy(this->vertices,vertices,sizeof(vertexid_t) * numEdges);
	memcpy(this->labels,labels,sizeof(char) * numEdges);
	memcpy(this->addr,addr,sizeof(vertexid_t) * numVertices);
	memcpy(this->index,index,sizeof(vertexid_t) * numVertices);
}

void Partition::repart(Partition &p,Context &c) {
	long totalNumEdges = c.vit.getDegree(this->id);
	int numPartitions = c.getNumPartitions();
	vertexid_t start = c.vit.getStart(this->id);
	vertexid_t end = c.vit.getEnd(this->id);

	long curNumEdges = 0;
	vertexid_t i;
	for(i = 0;i < numVertices;++i) {
		curNumEdges += index[i];
		if(curNumEdges >= totalNumEdges / 2) {	
			break;	
		}
	}

	// update p
	p.setId(numPartitions); 
	c.setNumPartitions(numPartitions+1);
	vertexid_t *pAddr = new vertexid_t[numVertices-i-1];
	for(vertexid_t j = 0;j < numVertices-1-i;++j) {
		pAddr[j] = addr[j+i+1]-addr[i+1];	// calculate offset
	}
	p.update(numVertices-1-i,totalNumEdges-curNumEdges,vertices+addr[i+1],labels+addr[i+1],pAddr,index+(i+1));

	delete[] pAddr;
	c.vit.add(start+i+1,end,totalNumEdges-curNumEdges);
	c.ddm.add();
	// update self
	numVertices = i+1; numEdges = curNumEdges;
	vertexid_t *tmpVertices = new vertexid_t[numEdges];	
	memcpy(tmpVertices,vertices,sizeof(vertexid_t) * numEdges); delete[] vertices; vertices = tmpVertices;	
	char *tmpLabels = new char[numEdges];
	memcpy(tmpLabels,labels,sizeof(char) * numEdges); delete[] labels; labels = tmpLabels;
	vertexid_t *tmpAddr = new vertexid_t[numVertices];
	memcpy(tmpAddr,addr,sizeof(vertexid_t) * numVertices); delete[] addr; addr = tmpAddr;
	vertexid_t *tmpIndex = new vertexid_t[numVertices];
	memcpy(tmpIndex,index,sizeof(vertexid_t) * numVertices); delete[] index; index = tmpIndex;
	c.vit.setVitValue(this->id,start,start+i,curNumEdges);
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

