<?xml version="1.0"?>
<library id="Course" type="physics">
 <scenedesc source="3dsmax">
  <eye>-30.7277 -71.5279 73.4541</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-30.4419 -70.8547 72.7721</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Box01">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-30.0241 29.9791 0.0</position>
   <shape id="shape_Box01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box01_geom">
     <convex facecount="0">
      <vertices count="8">
         -19.9074 -19.6978 0.0,
         19.9074 -19.6978 0.0,
         -19.9074 19.6978 0.0,
         19.9074 19.6978 0.0,
         -8.87567 -10.1638 1.39339,
         9.46457 -10.1638 1.39339,
         -8.87567 9.82928 1.39339,
         9.46457 9.82928 1.39339,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box02">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-31.0719 -29.5335 0.0</position>
   <shape id="shape_Box02">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box02_geom">
     <convex facecount="0">
      <vertices count="8">
         -19.6978 -20.1169 0.0,
         19.6978 -20.1169 0.0,
         -19.6978 20.1169 0.0,
         19.6978 20.1169 0.0,
         -6.98759 -9.89181 2.50517,
         8.41998 -9.89181 2.50517,
         -6.98759 5.90757 2.50517,
         8.41998 5.90757 2.50517,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box03">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>29.4885 29.3505 0.0</position>
   <shape id="shape_Box03">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box03_geom">
     <convex facecount="0">
      <vertices count="8">
         -19.9074 -20.3265 0.0,
         19.9074 -20.3265 0.0,
         -19.9074 20.3265 0.0,
         19.9074 20.3265 0.0,
         -7.34916 -9.02857 4.01966,
         8.89557 -9.02857 4.01966,
         -7.34916 9.70322 4.01966,
         8.89557 9.70322 4.01966,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box04">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>29.6981 -29.5335 0.0</position>
   <shape id="shape_Box04">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box04_geom">
     <convex facecount="0">
      <vertices count="8">
         -20.536 -20.1169 0.0,
         20.536 -20.1169 0.0,
         -20.536 20.1169 0.0,
         20.536 20.1169 0.0,
         -8.81497 -10.3109 2.44159,
         9.52211 -10.3109 2.44159,
         -8.81497 9.6795 2.44159,
         9.52211 9.6795 2.44159,
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
