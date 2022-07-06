<?xml version="1.0"?>
<library id="helloobjects" type="physics">
 <scenedesc source="3dsmax">
  <eye>24.7333 3.72135 12.1139</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>23.8533 3.58197 11.66</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Box01">
   <Mass>1.2</Mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-4.98917 -3.11821 2.08403</position>
   <shape>
    <Mass>1.2</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <physicsmaterial id="Box01_mat">
     <dynamicfriction>1.0</dynamicfriction>
     <restitution>0.7</restitution>
     <staticfriction>1.0</staticfriction>
    </physicsmaterial>
    <position>0.0 0.0 1.45253</position>
    <geometry id="Box01_geom">
     <box>
      <size>2.8707 4.36888 2.90506</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-0.0745544 2.8551 1.33624</position>
   <shape>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule01_geom">
     <capsule>
      <p0>0.0 0.0 0.835003</p0>
      <p1>0.0 0.0 3.45177</p1>
      <radius>0.835003</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Pyramid01">
   <Mass>3.0</Mass>
   <orientation>0.25391 0.348806 -0.0987655 0.896722</orientation>
   <position>-3.43686 3.63024 3.28707</position>
   <shape>
    <Mass>3.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Pyramid01_geom">
     <convex facecount="0">
      <vertices count="6">
         0.0 0.0 3.02682,
         -1.23259 -1.16993 0.0,
         1.23259 -1.16993 0.0,
         1.23259 1.16993 0.0,
         -1.23259 1.16993 0.0,
         0.0 0.0 0.0,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Torus01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-2.69445 3.99597 1.10461</position>
   <shape>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Torus01_geom">
     <trimesh>
      <vertices count="25">
         -1.32025e-007 3.02039 0.0,
         0.0 2.59697 0.582795,
         0.0 1.91185 0.360187,
         0.0 1.91185 -0.360187,
         0.0 2.59697 -0.582795,
         2.87256 0.933352 0.0,
         2.46986 0.802507 0.582795,
         1.81828 0.590794 0.360187,
         1.81828 0.590794 -0.360187,
         2.46986 0.802507 -0.582795,
         1.77534 -2.44355 0.0,
         1.52646 -2.10099 0.582795,
         1.12376 -1.54672 0.360187,
         1.12376 -1.54672 -0.360187,
         1.52646 -2.10099 -0.582795,
         -1.77534 -2.44355 0.0,
         -1.52646 -2.10099 0.582795,
         -1.12376 -1.54672 0.360187,
         -1.12376 -1.54672 -0.360187,
         -1.52646 -2.10099 -0.582795,
         -2.87256 0.933353 0.0,
         -2.46986 0.802507 0.582795,
         -1.81828 0.590794 0.360187,
         -1.81828 0.590794 -0.360187,
         -2.46986 0.802507 -0.582795,
      </vertices>
      <triangles count="50">
         0 6 5,
         0 1 6,
         1 7 6,
         1 2 7,
         2 8 7,
         2 3 8,
         3 9 8,
         3 4 9,
         4 5 9,
         4 0 5,
         5 11 10,
         5 6 11,
         6 12 11,
         6 7 12,
         7 13 12,
         7 8 13,
         8 14 13,
         8 9 14,
         9 10 14,
         9 5 10,
         10 16 15,
         10 11 16,
         11 17 16,
         11 12 17,
         12 18 17,
         12 13 18,
         13 19 18,
         13 14 19,
         14 15 19,
         14 10 15,
         15 21 20,
         15 16 21,
         16 22 21,
         16 17 22,
         17 23 22,
         17 18 23,
         18 24 23,
         18 19 24,
         19 20 24,
         19 15 20,
         20 1 0,
         20 21 1,
         21 2 1,
         21 22 2,
         22 3 2,
         22 23 3,
         23 4 3,
         23 24 4,
         24 0 4,
         24 20 0,
      </triangles>
     </trimesh>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Sphere01">
   <Mass>1.5</Mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-5.03286 1.11126 3.09494</position>
   <shape>
    <Mass>1.5</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>1.01562</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Hammer">
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>2.13619 -2.34641 1.80668</position>
   <shape>
    <Mass>28.3</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 -3.70953 0.0</position>
    <geometry id="head_geom">
     <box>
      <size>2.69204 0.999209 1.61892</size>
     </box>
    </geometry>
   </shape>
   <shape>
    <Mass>0.6</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>-0.0386279 0.322031 -0.00816725</position>
    <geometry id="handle_geom">
     <box>
      <size>0.352841 7.7742 0.365454</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="groundplane">
   <dynamic>0</dynamic>
   <Mass>0</Mass>
   <shape>
    <geometry>
     <plane>
      <d>0</d>
      <normal>0.0 0.0 1.0</normal>
     </plane>
    </geometry>
   </shape>
  </rigidbody>
 </physicsmodel>
</library>
