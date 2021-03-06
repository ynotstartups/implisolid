'use strict';




function __check_TypedArray_type(src, _type){
    //if(typeof src !== type_name)
    if(!(src instanceof _type))
     {
        console.error();
        throw "Developer's error; " + (typeof src) + (Object.prototype.toString.call(src.buffer));
    }
}

/**
* @param {Number | String} initial_value: if "random" then randomize color else if it's a number color set to this value
*/
function copy_Float32Array_preallocated(src, min_prealloc_size, initial_value)  {
    __check_TypedArray_type(src, Float32Array);
    var TYPE_SIZE = 4;
    if(min_prealloc_size % 1 !== 0) console.error("min_prealloc_size must be integer: " + min_prealloc_size);
    //if(min_prealloc_size !== int(min_prealloc_size)) console.error("min_prealloc_size must be integer: " + min_prealloc_size);
    var len_bytes = Math.max(min_prealloc_size*TYPE_SIZE, src.byteLength);
    var dst = new ArrayBuffer(len_bytes);
    var r = new Float32Array(dst);
    r.set(new Float32Array(src));
    if(initial_value === "random"){
        for(var i=0*src.byteLength/TYPE_SIZE;i<r.length;i++){
            r[i] = (Math.random()-0.5);
        }
    }else if (Number(initial_value) === initial_value){
        console.log("color number :" + initial_value + " " + len_bytes);
        for(var i=0*src.byteLength/TYPE_SIZE;i<r.length;i++){
            r[i] = initial_value;
        }

    }
    return r;
}
function copy_Uint32Array_preallocated(src, min_prealloc_size)  {
    __check_TypedArray_type(src, Uint32Array);
    var TYPE_SIZE = 4;
    //if(min_prealloc_size !== int(min_prealloc_size)) console.error("min_prealloc_size must be integer: " + min_prealloc_size);
    if(min_prealloc_size % 1 !== 0) console.error("min_prealloc_size must be integer: " + min_prealloc_size);
    var len_bytes = Math.max(min_prealloc_size*TYPE_SIZE, src.byteLength);
    var dst = new ArrayBuffer(len_bytes);
    // console.log("dst[0] : " + dst[0]);  // output:  undefined
    var r = new Uint32Array(dst);
    r.set(new Uint32Array(src));
    return r;
}
function copy_Uint16Array_preallocated(src, min_prealloc_size)  {
    __check_TypedArray_type(src, Uint16Array);
    var TYPE_SIZE = 2;
    if(min_prealloc_size !== int(min_prealloc_size)) console.error("min_prealloc_size must be integer: " + min_prealloc_size);
    var len_bytes = Math.max(min_prealloc_size*TYPE_SIZE, src.byteLength);;
    var dst = new ArrayBuffer(len_bytes);
    var r = new Uint16Array(dst);
    r.set(new Uint16Array(src));
    return r;
}

/** Simply creates a geometry . This is static and cannot be modified when displayed. Instantiate a new one and make a new THREE.Mesh() */
function LiveBufferGeometry71( verts_, faces_,  pre_allocate_, min_faces_capacity_, min_verts_capacity_) {

    THREE.BufferGeometry.call( this );
    this.type = 'LiveBufferGeometry71';

    this.parameters = { };

    if(min_faces_capacity_ === undefined) min_faces_capacity_ = 9000*0;
    if(min_verts_capacity_ === undefined) min_verts_capacity_ = 8000*0;

    const GROWTH_FACTOR = 1.5;
    const GROWTH_ADDITIONAL = 1;

    //_expect(min_faces_capacity_);
    //_expect(min_verts_capacity_);
    //console.log("verts : "+ min_verts_capacity_ + " faces : "+ min_faces_capacity_);

    this.allocate_buffers = function(verts, faces,  pre_allocate, faces_capacity, min_verts_capacity) {
        if(faces.length == 0){
            console.log("emptyimplicit");
            var verts = new Float32Array([0,0,0, 1,0,0, 1,1,0, 0,1,0, 0,0,1, 1,0,1, 1,1,1, 0,1,1 ]);
            var faces = new Uint32Array([0,1,2, 0,2,3, 0,4,5, 0,5,1, 1,5,6, 1,6,2, 2,6,3, 3,6,7, 4,5,6, 5,6,7]);
        }

        /*if (faces_capacity == 0 || faces.length == 0 || verts.length == 0 ) {
            console.warn("faces_capacity == 0");
        }
        */
        if (faces.length == 0 || verts.length == 0 ) {
            console.error("faces.length, verts.length == 0");
        }

        // console.log("min capacity for verts : "+ min_verts_capacity + " faces : "+ faces_capacity);
        var padded_faces, padded_verts;
        if(pre_allocate){
            // console.log("Allocating separate space for verts,faces.");
            padded_faces = copy_Uint32Array_preallocated(faces, faces_capacity*3);
            padded_verts = copy_Float32Array_preallocated(verts, min_verts_capacity*3);
        }
        else
        {
            // Warning: not cloned
            //padded_faces = faces;
            //padded_verts = verts;
            console.error("padded_faces = ?");
        }
        assert(pre_allocate);  // Will be wrong if pre_allocate is false

        // build geometry

        this.addAttribute( 'index', new THREE.BufferAttribute( padded_faces, 3 ) );
        this.addAttribute( 'position', new THREE.BufferAttribute( padded_verts, 3 ) );
        //this.setIndex( new THREE.BufferAttribute( padded_faces, 3 ) ); //new Uint32Array(padded_faces) ??
        for(var j=0;j<faces.length;j++)
            if(this.attributes.index.array[j] !== faces[j]){
                console.error(j);break;
            }
        //assert(this.attributes.index.array === padded_faces);

        //if(!ignoreNormals) {
        var padded_normals;
        if(pre_allocate){
            console.log("Allocating separate space for norms, colors.");
            padded_normals = copy_Float32Array_preallocated(verts, min_verts_capacity*3, "random");
            // if !ignoreNormals ...
            padded_normals.set(padded_verts);
            //var padded_colors = copy_Float32Array_preallocated(verts, min_verts_capacity*3, "random");
            //var uvs = copy_Float32Array_preallocated(new Float32Array([]), min_verts_capacity*3 * 0, "random");
            //padded_colors.set(padded_verts);
        }
        else{
            console.error("padded_normals = ?");
        }

        // We always need to allocate normals
        this.addAttribute( 'normal', new THREE.BufferAttribute( padded_normals, 3, true ) );

        //}

        //this.addAttribute( 'color', new THREE.BufferAttribute( padded_colors, 3, true ) ); //color is overidden
        //this.addAttribute( 'uv', new THREE.BufferAttribute( uvs, 2 ) );

        //var materialIndex = 0;
        //this.addGroup( 0, faces.length*1-10, materialIndex );

        var nf3 = faces.length;
        var gl_chunkSize=21845;

        /*
        var ii = 0;
        this.offsets.push({start:ii, index: ii , count: Math.min(nf3 - ii, gl_chunkSize*3)});
        */
        var triangles = nf3/3;
        this.offsets = [];
        var offsets = triangles / gl_chunkSize;
        for ( var i = 0; i < offsets; i ++ ) {

            var offset = {
                start: i * gl_chunkSize * 3,
                index: i * gl_chunkSize * 3,
                count: Math.min( triangles - ( i * gl_chunkSize ), gl_chunkSize ) * 3
            };

            this.offsets.push( offset );

        }
    }

    // ThreeJS does not use prototype-based OOP.
    /*
    This function may return new LiveBufferGeometry71 object, in this case the old geometry has to be replaced (by the new returned object) in the caller function.
    */
    this.update_geometry = function(implicit_service, ignoreNormals) {

        const _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;
        const _INT_SIZE = Uint32Array.BYTES_PER_ELEMENT;

        var geometry = this;
        var nverts = implicit_service.get_v_size();
        var nfaces = implicit_service.get_f_size();

        if(nfaces > 0){
            var verts_address = implicit_service.get_v_ptr();
            var faces_address = implicit_service.get_f_ptr();
            var verts = Module.HEAPF32.subarray(
                verts_address/_FLOAT_SIZE,
                verts_address/_FLOAT_SIZE + 3*nverts);
            var faces = Module.HEAPU32.subarray(
                faces_address/_INT_SIZE,
                faces_address/_INT_SIZE + 3*nfaces);
        }
        else{
            console.log("emptyimplicit");
            var verts = new Float32Array([0,0,0, 1,0,0, 1,1,0, 0,1,0, 0,0,1, 1,0,1, 1,1,1, 0,1,1 ]);
            var faces = new Uint32Array([0,1,2, 0,2,3, 0,4,5, 0,5,1, 1,5,6, 1,6,2, 2,6,3, 3,6,7, 4,5,6, 5,6,7]);
        }

        var g_nverts = geometry.attributes.position.array.length/3;  // Physical space size.
        var g_nfaces = geometry.attributes.index.array.length/3;

        var nv3 = Math.min(nverts, g_nverts) * 3;
        var nf3 = Math.min(nfaces, g_nfaces) * 3;
        //assert(nv3 === verts.length);
        //assert(nf3 === faces.length);

        // *************************************
        // * The following code works only when we use a single instance of the geometry, i.e. used in one Mesh.
        // * It will not work well if the same BufferGeometry is used in more than one Mesh
        // * So the object will look fine if we disable the wireframe mesh.
        // *************************************
        var growth_needed = false;
        var availableVertsSize = geometry.attributes.position.array.length;
        if(verts.length > availableVertsSize){

            growth_needed = true;

            //console.log("needed "+verts.length );
/*
            console.log(geometry.attributes.position.array.length);  */

            //geometry.attributes.position.array.set(verts.subarray(0, availableVertsSize));


/*
            var reserved_size = (verts.length/3) * 2;
            console.log("reserved "+reserved_size);
            var new_pos_array = copy_Float32Array_preallocated(verts, reserved_size*3);
            geometry.attributes.position.array = new_pos_array;
            //this.addAttribute( 'position', new THREE.BufferAttribute( new_pos_array, 3 ) );
            geometry.attributes.position.array.set(verts);


            var new_pos_array = copy_Float32Array_preallocated(verts, reserved_size*3);
            geometry.attributes.normal.array = new_pos_array;
            geometry.attributes.normal.array.set(verts);

            //var new_pos_array = copy_Float32Array_preallocated(verts, reserved_size*3);
            //geometry.attributes.color = new_pos_array;
            //geometry.attributes.color.set(verts);

            //var positionAttr = geometry.attributes.position;
            //positionAttr.updateRange.offset = 0; // where to start updating
            //positionAttr.updateRange.count = 1;

            //The problem is geometry.attributes.position.buffer is not updated.


            console.log(geometry.attributes.position.array.length);
            */
        }else{
            geometry.attributes.position.array.set(verts);
            if(!ignoreNormals) {
                geometry.attributes.normal.array.set(verts);
            }
        }

        var availableFacesSize = geometry.attributes.index.array.length;
        if(faces.length > availableFacesSize){
            //geometry.attributes.index.array.set(faces.subarray(0, availableFacesSize));
            growth_needed = true;
        }else{
            geometry.attributes.index.array.set(faces);
        }

        if(growth_needed){
            console.log("increasing capacity : availableFacesSize : " + availableFacesSize + " facesLength : " + faces.length);
            //this.dispose();
            var faces_capacity = Math.floor(Math.max(availableFacesSize/3, faces_.length/3) * GROWTH_FACTOR + GROWTH_ADDITIONAL);
            var min_verts_capacity = Math.floor(Math.max(availableVertsSize/3, verts_.length/3) * GROWTH_FACTOR + GROWTH_ADDITIONAL);

            console.error(faces_capacity+"  "+min_verts_capacity);

            this.allocate_buffers(verts, faces,  true, faces_capacity, min_verts_capacity);
            //var new_geometry= new LiveBufferGeometry71( verts, faces,  true, Math.max(availableFacesSize/3, faces.length/3) * 1.5 + 1, Math.max(availableVertsSize/3, verts.length/3) * 1.5 +1);

            geometry.attributes.position.needsUpdate = true;
            geometry.attributes.index.needsUpdate = true;
            if(!ignoreNormals) {
                geometry.attributes.normal.needsUpdate = true;
            }

            return false;//new_geometry;

        }
        var copied_faces = Math.min(faces.length, availableFacesSize);

        //copied_faces = nf3;

        //geometry.setDrawRange( 0, nf3 );
        //geometry.attributes.index.length = nf3;

        /*
        var ii = 0;
        var gl_chunkSize=21845;
        geometry.offsets = [];
        geometry.offsets.push({start:ii, index: ii , count: Math.min( copied_faces - ii, gl_chunkSize*3)});
        //FIXME: when many vertices
        _expect(copied_faces - ii <= gl_chunkSize*3);
        */

        var triangles = copied_faces/3;
        var gl_chunkSize=21845;
        geometry.offsets = [];
        var offsets = triangles / gl_chunkSize;
        for ( var i = 0; i < offsets; i ++ ) {

            var offset = {
                start: i * gl_chunkSize * 3,
                index: i * gl_chunkSize * 3,
                count: Math.min( triangles - ( i * gl_chunkSize ), gl_chunkSize ) * 3
            };

            geometry.offsets.push( offset );

        }
        //geometry.computeBoundingSphere();


        /*geometry.clearGroups();
        geometry.addGroup( 0, nf3, 0 );
        */



        //geometry.computeOffsets();
        geometry.attributes.position.needsUpdate = true;
        geometry.attributes.index.needsUpdate = true;
        if(!ignoreNormals) {
            geometry.attributes.normal.needsUpdate = true;
        }
        //geometry.attributes.color.needsUpdate = true;

        /*
        g =currentMeshes[0].geometry

        IMPLICIT.finish_geometry();
        var mc_properties_json = JSON.stringify({resolution: CONFIG.implisolid.default_mc_resolution, box: {xmin: -1, xmax: 1, ymin: -1 , ymax: 1, zmin: -1, zmax: 1}});
        IMPLICIT.build_geometry(......., mc_properties_json,);
        // g.update_geometry(IMPLICIT, false)
        IMPLICIT.update_geometry(g, false)


        for(var i=0;i<1000;i++){ IMPLICIT.finish_geometry();IMPLICIT.build_geometry(..........28, mc_properties_json, "sphere", i*0.1); g.update_geometry(IMPLICIT, false);}

        */
        return false;




    };

    this.update_normals_from_array = function(normals) {
        //i.e. setNormals()
        // Only called after updateGeometry(), or after LiveGeometry()
        var geometry = this;
        // assert ensure length
        geometry.attributes.normal.array.set(normals);
        geometry.attributes.normal.needsUpdate = true;
    };

    // just for backward comptibility
    this.update_normals = function(implicit_service, x, mp5_str, ignore_root_matrix) {
        // Only called after updateGeometry(), or after LiveGeometry()
        console.error("error. update_normals()");
        throw new Error("Deprecated. You should not call this");

        implicit_service.make_normals_into_geometry(geom, mp5_str, x, ignore_root_matrix);

        //this.aaaaaaaaaA(x, mp5_str, ignore_root_matrix) {

        // var x = new Float32Array(nverts);

        /*
        const _FLOAT_SIZE = Float32Array.BYTES_PER_ELEMENT;

        implicit_service.set_object(mp5_str, ignore_root_matrix);
        implicit_service.set_vect(x);  // overhead
        implicit_service.calculate_implicit_gradients(true);
        var ptr = implicit_service.get_gradients_ptr();
        var ptr_len = implicit_service.get_gradients_size();
        var gradients = Module.HEAPF32.subarray(ptr/_FLOAT_SIZE, ptr/_FLOAT_SIZE + ptr_len);
        //console.log("grad len = " +  ptr_len+ "  grad = " + gradients);  // x 4

        var geom = this;
        geom.update_normals_from_array(gradients);

        implicit_service.unset_x();
        implicit_service.unset_object();
        */
    };

    this.use_default_normals_from_vertices = function () {
        // use default normals
        // candidate name: remove_normals()

        console.warn("not tested for veriosn prior to r79");

        this.removeAttribute('normal');
        this.computeVertexNormals();
        this.__set_needsUpdate_flag(false);
    }

    this.get_minz = function(matrix, shape) {
        // shape is not needed really.
        return +1;
    }


    // this.computeBoundingBox = function...; // not needed.
    // this.computeBoundingSphere = function...; // not needed.

    this.allocate_buffers(verts_, faces_,  pre_allocate_, min_faces_capacity_, min_verts_capacity_);
};

LiveBufferGeometry71.prototype = Object.create( THREE.BufferGeometry.prototype );
LiveBufferGeometry71.prototype.constructor = LiveBufferGeometry71;
