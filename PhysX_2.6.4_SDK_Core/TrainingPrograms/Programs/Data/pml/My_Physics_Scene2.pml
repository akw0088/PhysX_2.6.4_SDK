<?xml version="1.0"?>
<library id="My_Physics_Scene2" type="physics">
 <scenedesc source="3dsmax">
  <eye>-10.2062 -10.2062 8.33333</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-9.59383 -9.59383 7.83333</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Sphere01">
   <dynamic>0</dynamic>
   <mass>0.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-3.08224 3.93482 1.00754</position>
   <shape id="shape_Sphere01">
    <mass>0.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.992328</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box01">
   <dynamic>0</dynamic>
   <mass>0.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-3.11802 -3.6841 0.0</position>
   <shape id="shape_Box01">
    <mass>0.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.654912</position>
    <geometry id="Box01_geom">
     <box>
      <size>1.30982 1.71285 1.30982</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Prism01">
   <dynamic>0</dynamic>
   <mass>0.0</mass>
   <orientation>0.707107 0.0 0.0 0.707107</orientation>
   <position>1.91786 2.12286 0.0</position>
   <shape id="shape_Prism01">
    <mass>0.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Prism01_geom">
     <convex facecount="0">
      <vertices count="8">
         6.18904 0.402269 4.04534,
         0.0 0.0 4.04534,
         12.5441 0.0 4.04534,
         6.02304 1.20681 4.04534,
         0.0 0.0 0.0,
         12.5441 0.0 0.0,
         6.02304 1.20681 0.0,
         6.18904 0.402269 0.0,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="groundplane">
   <dynamic>0</dynamic>
   <mass>0</mass>
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
