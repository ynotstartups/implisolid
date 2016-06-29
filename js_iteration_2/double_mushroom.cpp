class double_mushroom : public implicit_function {

protected:
    REAL r;

public:
    double_mushroom(REAL r){
        this->r = r;
        //works with r>3********
    }

    //boost::array<int, 2> big_shape = {{ 10000, 3 }};
    //boost::multi_array<REAL, 2> huge_test =  boost::multi_array<REAL, 2>(big_shape);

    void eval_implicit(const vectorized_vect& x, vectorized_scalar& f_output){
        my_assert(assert_implicit_function_io(x, f_output), "");
        my_assert(this->integrity_invariant(), "");

        const REAL r2 = squared(this->r);
        //auto i = x.begin();
        int output_ctr=0;
        //const vectorized_vect::iterator
        auto i = x.begin();
        auto e = x.end();
        for(; i<e; i++, output_ctr++){
            (f_output)[output_ctr] = -(pow((*i)[0],2)*r2+pow((*i)[1],2)*r2-pow((*i)[2],2)*r2-1);
            if((*i)[2]>0.8) (f_output)[output_ctr]=-1;
            if((*i)[2]<-0.8) (f_output)[output_ctr]=-1;

        }
    }
    void eval_gradient(const vectorized_vect& x, vectorized_vect& output){
        //(*output) = x;
        const REAL r2 = squared(this->r);
        int output_ctr=0;
        auto i = x.begin();
        auto e = x.end();
        for(; i<e; i++, output_ctr++){
            (output)[output_ctr][0] = -r2 * (*i)[0];
            (output)[output_ctr][1] = -r2 * (*i)[1];
            (output)[output_ctr][2] = r2 * (*i)[2];
            if((*i)[2]<0.8 && (*i)[2]>0.78 && pow((*i)[0],2)+pow((*i)[1],2)<0.9) {(output)[output_ctr][0] = 0;
                        (output)[output_ctr][1] = 0;
                        (output)[output_ctr][2] = -1;}
            if((*i)[2]>0.80) {(output)[output_ctr][0] = 0;
                        (output)[output_ctr][1] = 0;
                        (output)[output_ctr][2] = -1;}
            if((*i)[2]>-0.8 && (*i)[2]<-0.78 && pow((*i)[0],2)+pow((*i)[1],2)<0.9) {(output)[output_ctr][0] = 0;
                        (output)[output_ctr][1] = 0;
                        (output)[output_ctr][2] = 1;}
            if((*i)[2]<-0.80) {(output)[output_ctr][0] = 0;
                        (output)[output_ctr][1] = 0;
                        (output)[output_ctr][2] = 1;}
        }
    }
    bool integrity_invariant(){
        return true;
    }
};
