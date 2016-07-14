/**
* File: test_crisp_subtract.cpp
*------------------------------
*
* In this file there are tests for the crisp subtraction operation.
*/

#include "../mcc2.cpp"
#include "gtest/gtest.h"
#include <string.h>

#define EPSILON 0.001

/********************************************************/
/************* utils functions to tests *****************/
/********************************************************/

template<typename T> inline const T abs(T const & x)
{
    return ( x<0 ) ? -x : x;
}


mp5_implicit::bounding_box generate_random_box(mp5_implicit::bounding_box givenBoundingBox){
    mp5_implicit::bounding_box resultBox;

    resultBox.xmin = (rand() % (int)((givenBoundingBox.xmax - givenBoundingBox.xmin + 1)*1000) + givenBoundingBox.xmin * 1000) / 1000.0 ;
    resultBox.xmax = (rand() % (int)((givenBoundingBox.xmax - resultBox.xmin + 1)*1000) + resultBox.xmin *1000 ) / 1000.0  ;

    resultBox.ymin = (rand() % (int)((givenBoundingBox.ymax - givenBoundingBox.ymin + 1)*1000) + givenBoundingBox.ymin * 1000) / 1000.0 ;
    resultBox.ymax = (rand() % (int)((givenBoundingBox.ymax - resultBox.ymin + 1)*1000) + resultBox.ymin *1000 ) / 1000.0 ;

    resultBox.zmin = (rand() % (int)((givenBoundingBox.zmax - givenBoundingBox.zmin + 1)*1000) + givenBoundingBox.zmin *1000 ) / 1000.0 ;
    resultBox.zmax = (rand() % (int)((givenBoundingBox.zmax - resultBox.zmin + 1)*1000) + resultBox.zmin * 1000 ) / 1000.0 ;


    return resultBox;
}

void get_real_boundingBox(mp5_implicit::bounding_box& box){
    if(!check_state())
    return;
    //int nf = get_f_size();
    for(std::vector<REAL>::iterator it=_state.mc->result_verts.begin(); it < _state.mc->result_verts.end(); it+=3 ){
        REAL currentval = *( it );
        if(currentval < box.xmin ){
            box.xmin = currentval;
        }else if(currentval > box.xmax){
            box.xmax = currentval;
        }

        currentval = *( it + 1 );
        if(currentval < box.ymin ){
            box.ymin = currentval;
        }else if(currentval > box.ymax){
            box.ymax = currentval;
        }

        currentval = *( it + 2 );
        if(currentval < box.zmin ){
            box.zmin = currentval;
        }else if(currentval > box.zmax){
            box.zmax = currentval;
        }

    }
}

/********************************************************/
/********************************************************/


/********************************************************/
/********* TESTING EGG (exact bounding box ) ************/
/********************************************************/

TEST(MarchingCubesBoundingBox, ExactBoundingBoxEgg) {
/**
 * Description: testing with the exact bounding box of egg
 */


    build_geometry("{\"type\":\"egg\",\"displayColor\":{\"x\":0.6413270673767635,\"y\":0.8385504499196117,\"z\":0.4723932649315161},\"matrix\":[10,0,0,0,0,10,0,0,0,0,10,0,0,0,0,1],\"index\":7920505}",
        "{\"resolution\":28,\"box\":{\"xmin\":-5,\"xmax\":5,\"ymin\":-5,\"ymax\":5,\"zmin\":-5,\"zmax\":5}}");

    EXPECT_EQ(get_f_size(),7352);

    mp5_implicit::bounding_box box = {10,-10,10,-10,10,-10};

    get_real_boundingBox(box);
    //check min and max
    EXPECT_GE(box.xmin,-5);
    EXPECT_GE(box.ymin,-5);
    EXPECT_GE(box.zmin,-5);

    EXPECT_LE(box.xmax,5);
    EXPECT_LE(box.ymax,5);
    EXPECT_LE(box.zmax,5);




    finish_geometry();
}
/********************************************************/
/********************************************************/


/********************************************************/
/**** TESTING SIMPLE_SPHERE (exact bounding box ) *******/
/********************************************************/

TEST(MarchingCubesBoundingBox, ExactBoundingBoxSphere) {


    build_geometry("{\"type\":\"simple_sphere\", \"radius\": 3.0}",
        "{\"resolution\":28,\"box\":{\"xmin\":-3,\"xmax\":3,\"ymin\":-3,\"ymax\":3,\"zmin\":-3,\"zmax\":3}}");

    EXPECT_EQ(get_f_size(),7352);

    mp5_implicit::bounding_box box = {10,-10,10,-10,10,-10};
    get_real_boundingBox(box);
    //check min and max
    EXPECT_GE(box.xmin,-5);
    EXPECT_GE(box.ymin,-5);
    EXPECT_GE(box.zmin,-5);

    EXPECT_LE(box.xmax,5);
    EXPECT_LE(box.ymax,5);
    EXPECT_LE(box.zmax,5);



    finish_geometry();
}
/********************************************************/
/********************************************************/

/********************************************************/
/*TESTING IMPLICIT_DOUBLE_MUSHROOM  (exact bounding box)*/
/********************************************************/
TEST(MarchingCubesBoundingBox, ExactBoundingBoxSphereDoubleMushroom) {



    build_geometry("{\"type\":\"implicit_double_mushroom\",\"displayColor\":{\"x\":0.10352949217446406,\"y\":0.27710496720866984,\"z\":0.23427879298291177},\"matrix\":[10,0,0,0,0,10,0,0,0,0,10,0,0,0,0,1],\"index\":7584363}",
    "{\"resolution\":28,\"box\":{\"xmin\":-1,\"xmax\":1,\"ymin\":-1,\"ymax\":1,\"zmin\":-1,\"zmax\":1}}");

    EXPECT_EQ(get_f_size(),5880);

    mp5_implicit::bounding_box box = {10,-10,10,-10,10,-10};
    get_real_boundingBox(box);
    //check min and max
    EXPECT_GE(box.xmin,-5);
    EXPECT_GE(box.ymin,-5);
    EXPECT_GE(box.zmin,-5);

    EXPECT_LE(box.xmax,5);
    EXPECT_LE(box.ymax,5);
    EXPECT_LE(box.zmax,5);



    finish_geometry();

}
/********************************************************/
/********************************************************/



TEST(MarchingCubesBoundingBox, RandomBoundingBoxEgg) {

    mp5_implicit::bounding_box givenBox = {-5,5,-5,5,-5,5};
    mp5_implicit::bounding_box randomBox;


    for(int i = 0; i < 10 ; i++){

        randomBox = generate_random_box(givenBox);

        std::stringstream mc_params_json ;
        mc_params_json << "{\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" ;
         build_geometry("{\"type\":\"egg\",\"displayColor\":{\"x\":0.6413270673767635,\"y\":0.8385504499196117,\"z\":0.4723932649315161},\"matrix\":[10,0,0,0,0,10,0,0,0,0,10,0,0,0,0,1],\"index\":7920505}",

        strdup(mc_params_json.str().c_str()));

        mp5_implicit::bounding_box realBox = {10,-10,10,-10,10,-10};
        get_real_boundingBox(realBox);

        //std::cout << " random : {\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" << std::endl;

        //std::cout << " real : {\"resolution\":28,\"box\":{\"xmin\":" << realBox.xmin << ",\"xmax\":" << realBox.xmax << ",\"ymin\":" << realBox.ymin << ",\"ymax\":" << realBox.ymax << ",\"zmin\":" << realBox.zmin << ",\"zmax\":" << realBox.zmax << "}}" << std::endl;

        // //check min and max
        EXPECT_GE(realBox.xmin-randomBox.xmin,-EPSILON);
        EXPECT_GE(realBox.ymin-randomBox.ymin,-EPSILON);
        EXPECT_GE(realBox.zmin-randomBox.zmin,-EPSILON);
        EXPECT_GE(randomBox.xmax-realBox.xmax,-EPSILON);
        EXPECT_GE(randomBox.ymax-realBox.ymax,-EPSILON);
        EXPECT_GE(randomBox.zmax-realBox.zmax,-EPSILON);


        finish_geometry();

    }

}


TEST(MarchingCubesBoundingBox, RandomBoundingBoxSimpleSphere) {

    mp5_implicit::bounding_box givenBox = {-3,3,-3,3,-3,3};
    mp5_implicit::bounding_box randomBox;


    for(int i = 0; i < 10 ; i++){

        randomBox = generate_random_box(givenBox);

        std::stringstream mc_params_json ;
        mc_params_json << "{\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" ;
         build_geometry("{\"type\":\"simple_sphere\", \"radius\": 3.0}",
        strdup(mc_params_json.str().c_str()));

        mp5_implicit::bounding_box realBox = {10,-10,10,-10,10,-10};
        get_real_boundingBox(realBox);

        //std::cout << " random : {\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" << std::endl;

        //std::cout << " real : {\"resolution\":28,\"box\":{\"xmin\":" << realBox.xmin << ",\"xmax\":" << realBox.xmax << ",\"ymin\":" << realBox.ymin << ",\"ymax\":" << realBox.ymax << ",\"zmin\":" << realBox.zmin << ",\"zmax\":" << realBox.zmax << "}}" << std::endl;

        // //check min and max
        EXPECT_GE(realBox.xmin-randomBox.xmin,-EPSILON);
        EXPECT_GE(realBox.ymin-randomBox.ymin,-EPSILON);
        EXPECT_GE(realBox.zmin-randomBox.zmin,-EPSILON);
        EXPECT_GE(randomBox.xmax-realBox.xmax,-EPSILON);
        EXPECT_GE(randomBox.ymax-realBox.ymax,-EPSILON);
        EXPECT_GE(randomBox.zmax-realBox.zmax,-EPSILON);


        finish_geometry();

    }
}

TEST(MarchingCubesBoundingBox, RandomBoundingBoxDoubleMushroom) {

    mp5_implicit::bounding_box givenBox = {-1,1,-1,1,-1,1};
    mp5_implicit::bounding_box randomBox;


    for(int i = 0; i < 10 ; i++){

        randomBox = generate_random_box(givenBox);

        std::stringstream mc_params_json ;
        mc_params_json << "{\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" ;
         build_geometry("{\"type\":\"implicit_double_mushroom\",\"displayColor\":{\"x\":0.10352949217446406,\"y\":0.27710496720866984,\"z\":0.23427879298291177},\"matrix\":[10,0,0,0,0,10,0,0,0,0,10,0,0,0,0,1],\"index\":7584363}",

        strdup(mc_params_json.str().c_str()));

        mp5_implicit::bounding_box realBox = {10,-10,10,-10,10,-10};
        get_real_boundingBox(realBox);

        //std::cout << " random : {\"resolution\":28,\"box\":{\"xmin\":" << randomBox.xmin << ",\"xmax\":" << randomBox.xmax << ",\"ymin\":" << randomBox.ymin << ",\"ymax\":" << randomBox.ymax << ",\"zmin\":" << randomBox.zmin << ",\"zmax\":" << randomBox.zmax << "}}" << std::endl;

        //std::cout << " real : {\"resolution\":28,\"box\":{\"xmin\":" << realBox.xmin << ",\"xmax\":" << realBox.xmax << ",\"ymin\":" << realBox.ymin << ",\"ymax\":" << realBox.ymax << ",\"zmin\":" << realBox.zmin << ",\"zmax\":" << realBox.zmax << "}}" << std::endl;

        // //check min and max
        EXPECT_GE(realBox.xmin-randomBox.xmin,-EPSILON);
        EXPECT_GE(realBox.ymin-randomBox.ymin,-EPSILON);
        EXPECT_GE(realBox.zmin-randomBox.zmin,-EPSILON);
        EXPECT_GE(randomBox.xmax-realBox.xmax,-EPSILON);
        EXPECT_GE(randomBox.ymax-realBox.ymax,-EPSILON);
        EXPECT_GE(randomBox.zmax-realBox.zmax,-EPSILON);


        finish_geometry();

    }

}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "Good bye." << std::endl;
    return RUN_ALL_TESTS();
}