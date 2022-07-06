<?xml version="1.0"?>
<library id="myphysicsscene2" type="physics">
 <scenedesc source="3dsmax">
  <eye>-15.3093 -15.3093 12.5</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-14.6969 -14.6969 12.0</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Sphere01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-2.88174 4.06123 1.00754</position>
   <shape id="shape_Sphere01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.99743</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-3.15103 -2.7204 0.0</position>
   <shape id="shape_Box01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.906801</position>
    <geometry id="Box01_geom">
     <box>
      <size>1.91436 1.71285 1.8136</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Pyramid01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>3.0658 3.99381 0.0005</position>
   <shape id="shape_Pyramid01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Pyramid01_geom">
     <convex facecount="0">
      <vertices count="6">
         0.0 0.0 2.99335,
         -1.35218 -0.88486 0.0,
         1.35218 -0.88486 0.0,
         1.35218 0.88486 0.0,
         -1.35218 0.88486 0.0,
         0.0 0.0 0.0,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>3.22418 -2.86227 0.0</position>
   <shape id="shape_Capsule01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule01_geom">
     <capsule>
      <p0>0.0 0.0 0.645151</p0>
      <p1>0.0 0.0 1.65271</p1>
      <radius>0.645151</radius>
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
