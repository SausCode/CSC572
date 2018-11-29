


#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>






class Program;

class Shape
{

public:
	//stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
	void Shape::drawBasic(const std::shared_ptr<Program> prog) const;
	void loadMesh(const std::string &meshName, std::string *mtlName = NULL, unsigned char *(loadimage)(char const *, int *, int *, int *, int) = NULL);
	void init();
	void resize();
	void draw(const std::shared_ptr<Program> prog, bool use_extern_texures) const;
	unsigned int *textureIDs = NULL;
	unsigned int *normalmapsIDs = NULL;
	
	void calc_SxT();
	std::vector<unsigned int> *eleBuf = NULL;
	std::vector<float> *posBuf = NULL;
	std::vector<float> *norBuf = NULL;
	std::vector<float> *texBuf = NULL;
	std::vector<float> *tanBuf = NULL;
	std::vector<float> *binormBuf = NULL;
	int obj_count = 0;
private:
	
	
	unsigned int *materialIDs = NULL;


	unsigned int *eleBufID = 0;
	unsigned int *posBufID = 0;
	unsigned int *norBufID = 0;
	unsigned int *texBufID = 0;
	unsigned int *tanBufID = 0;
	unsigned int *binormBufID = 0;
	unsigned int *vaoID = 0;

};
void computeTangentSpace(Shape *shape);
#endif // LAB471_SHAPE_H_INCLUDED
