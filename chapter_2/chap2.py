
# sample code to illustrate how to use BlendFileReader.py, and the blend file; Chapter Two
# test this on the "on-disk" startup.blend file
# usage: run from doc/

# What the script does:
# 1. list basic header and version info
# 2. list number of file-blocks, names, types, and structs
# 3. finds and displays information about a material from a mesh object

import BlendFileReader as bfr

BLENDFILE = '../../release/datafiles/startup.blend' # pwd is doc/
decomp_handle = bfr.openBlendFile(BLENDFILE)
startup = bfr.BlendFile(decomp_handle)

print('Version: {}'.format(startup.Header.Version)) # blender version of .blend file...

# first 10 variables names in dna
for name in startup.Catalog.Names[0:10]:
    print('Name in SDNA Catalog: {}'.format(name.Name))
    
# first 10 DNA types (i.e., dna structures & primitives types)
for type in startup.Catalog.Types[0:10]:
    print('Type in SDNA Catalog: {}'.format(type.Name))

    
print('# of Blocks in startup: {}'.format(len(startup.Blocks)))
print('# of Names in startup SDNA: {}'.format(len(startup.Catalog.Names)))
print('# of Types in startup SDNA: {}'.format(len(startup.Catalog.Types)))
print('# of Structs in startup SDNA: {}'.format(len(startup.Catalog.Structs)))

# find mesh with material
for blocknum, block in enumerate(startup.Blocks):
    sdna_idx = block.Header.SDNAIndex
    dna_type = startup.Catalog.Structs[sdna_idx].Type.Name
    if dna_type == 'Mesh':
        print('Block {}\n\tHeader code: {}'.format(blocknum, block.Header.Code))
        print('\tSNDA index: {}'.format(sdna_idx))
        print('\tType name: {}'.format(dna_type))
        print('\tOld address: {}'.format(startup.Blocks[blocknum].Header.OldAddress))
        mesh = startup.Blocks[blocknum]
        # find block with material pointer link, see DNA struct at end of file for ref.
        # or in source/blender/makesdna/DNA_mesh_types.h
        mat_ptr_ptr = mesh.Get(decomp_handle, 'mat')
        print('\tstruct Material **mat: {}'.format(mat_ptr_ptr))
        for blocknum2, block2 in enumerate(startup.Blocks):
            if block2.Header.OldAddress == mat_ptr_ptr:
                #for block in startup.Blocks:
                sdna_idx2 = block2.Header.SDNAIndex
                dna_type2 = startup.Catalog.Structs[sdna_idx2].Type.Name
                print('\tBlock {}\n\t\tHeader code: {}'.format(blocknum2, block2.Header.Code))
                print('\t\tSNDA index: {}'.format(sdna_idx2))
                print('\t\tType name: {}'.format(dna_type2))
                print('\t\tOld address: {}'.format(startup.Blocks[blocknum2].Header.OldAddress))
                link = startup.Blocks[blocknum2]
                mat_ptr = link.Get(decomp_handle, 'next')
                for blocknum3, block3 in enumerate(startup.Blocks):
                    if block3.Header.OldAddress == mat_ptr:
                        sdna_idx3 = block3.Header.SDNAIndex
                        dna_type3 = startup.Catalog.Structs[sdna_idx3].Type.Name
                        print('\t\tBlock {}\n\t\t\tHeader code: {}'.format(blocknum3, block3.Header.Code))
                        print('\t\t\tSNDA index: {}'.format(sdna_idx3))
                        print('\t\t\tType name: {}'.format(dna_type3))
                        print('\t\t\tOld address: {}'.format(startup.Blocks[blocknum3].Header.OldAddress))
                        mat = startup.Blocks[blocknum3]
                        red = mat.Get(decomp_handle, 'r')
                        green = mat.Get(decomp_handle, 'g')
                        blue = mat.Get(decomp_handle, 'b')
                        print('\t\t\tRed component {}'.format(red))
                        print('\t\t\tGreen component {}'.format(green))
                        print('\t\t\tBlue component {}'.format(blue))
                       

# from DNA_mesh_types.h
'''
Above Python script finds a Material object associated with
the first found struct Mesh object. Illustrates how to find a particular file-block serialization
of a DNA struct object's field. See comment below.

typedef struct Mesh {
  ID id;
  /** Animation data (must be immediately after id for utilities to use it). */
  struct AnimData *adt;

  /** Old animation system, deprecated for 2.5. */
  struct Ipo *ipo DNA_DEPRECATED;
  struct Key *key;
  struct Material **mat; // <--- finding this field here, the first item in an array of materials
  struct MSelect *mselect;

  /* BMESH ONLY */
  /*new face structures*/
  struct MPoly *mpoly;
  struct MLoop *mloop;
  struct MLoopUV *mloopuv;
  struct MLoopCol *mloopcol;
  /* END BMESH ONLY */

  /* mface stores the tessellation (triangulation) of the mesh,
   * real faces are now stored in nface.*/
  /** Array of mesh object mode faces for tessellation. */
  struct MFace *mface;
  /** Store tessellation face UV's and texture here. */
  struct MTFace *mtface;
  /** Deprecated, use mtface. */
  struct TFace *tface DNA_DEPRECATED;
  /** Array of verts. */
  struct MVert *mvert;
  /** Array of edges. */
  struct MEdge *medge;
  /** Deformgroup vertices. */
  struct MDeformVert *dvert;

  /* array of colors for the tessellated faces, must be number of tessellated
   * faces * 4 in length */
  struct MCol *mcol;
  struct Mesh *texcomesh;

  /* When the object is available, the preferred access method is: BKE_editmesh_from_object(ob) */
  /** Not saved in file!. */
  struct BMEditMesh *edit_mesh;

  struct CustomData vdata, edata, fdata;

  /* BMESH ONLY */
  struct CustomData pdata, ldata;
  /* END BMESH ONLY */

  int totvert, totedge, totface, totselect;

  /* BMESH ONLY */
  int totpoly, totloop;
  /* END BMESH ONLY */

  /* the last selected vertex/edge/face are used for the active face however
   * this means the active face must always be selected, this is to keep track
   * of the last selected face and is similar to the old active face flag where
   * the face does not need to be selected, -1 is inactive */
  int act_face;

  /* texture space, copied as one block in editobject.c */
  float loc[3];
  float size[3];

  short texflag, flag;
  float smoothresh;

  /* customdata flag, for bevel-weight and crease, which are now optional */
  char cd_flag, _pad;

  char subdiv DNA_DEPRECATED, subdivr DNA_DEPRECATED;
  /** Only kept for backwards compat, not used anymore. */
  char subsurftype DNA_DEPRECATED;
  char editflag;

  short totcol;

  float remesh_voxel_size;
  float remesh_voxel_adaptivity;
  char remesh_mode;
  char _pad1[3];
  /** Deprecated multiresolution modeling data, only keep for loading old files. */
  struct Multires *mr DNA_DEPRECATED;

  Mesh_Runtime runtime;
} Mesh;
'''


            
        
    



