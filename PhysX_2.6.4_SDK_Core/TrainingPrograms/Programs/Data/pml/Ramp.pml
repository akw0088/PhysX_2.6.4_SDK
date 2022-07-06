<?xml version="1.0"?>
<library id="Ramp" type="physics">
 <scenedesc source="3dsmax">
  <eye>-10.2062 -10.2062 8.33333</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-9.59383 -9.59383 7.83333</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Sphere01">
   <mass>50.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-3.02211 2.98457 0.961473</position>
   <shape id="shape_Sphere01">
    <mass>50.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.871173</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box01">
   <mass>50.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-3.03855 -2.94785 0.0</position>
   <shape id="shape_Box01">
    <mass>50.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.702948</position>
    <geometry id="Box01_geom">
     <box>
      <size>1.63265 1.54195 1.4059</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box02">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>5.37415 -0.0226758 0.0</position>
   <shape id="shape_Box02">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box02_geom">
     <convex facecount="0">
      <vertices count="8">
         -3.51474 -2.15419 0.0,
         3.51474 -2.15419 0.0,
         -3.51474 2.15419 0.0,
         3.51474 2.15419 0.0,
         -1.06884 -2.15419 1.08844,
         1.31885 -2.15419 1.08844,
         -1.06884 2.15419 1.08844,
         1.31885 2.15419 1.08844,
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
