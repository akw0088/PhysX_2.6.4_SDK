<?xml version="1.0"?>
<library id="My_Physics_Scene" type="physics">
 <scenedesc source="3dsmax">
  <eye>-15.3093 -15.3093 12.5</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-14.6969 -14.6969 12.0</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Box01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-2.7124 -2.73271 0.0</position>
   <shape id="shape_Box01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.957179</position>
    <geometry id="Box01_geom">
     <box>
      <size>1.00756 1.51134 1.91436</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Sphere01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-2.84832 2.93876 1.18219</position>
   <shape id="shape_Sphere01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>1.00756</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Pyramid01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>2.53504 2.85954 0.0005</position>
   <shape id="shape_Pyramid01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Pyramid01_geom">
     <convex facecount="0">
      <vertices count="6">
         0.0 0.0 2.21662,
         -0.856423 -0.856423 0.0,
         0.856423 -0.856423 0.0,
         0.856423 0.856423 0.0,
         -0.856423 0.856423 0.0,
         0.0 0.0 0.0,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>2.61582 -2.77911 0.0</position>
   <shape id="shape_Capsule01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule01_geom">
     <capsule>
      <p0>0.0 0.0 0.726559</p0>
      <p1>0.0 0.0 2.03638</p1>
      <radius>0.726559</radius>
     </capsule>
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
