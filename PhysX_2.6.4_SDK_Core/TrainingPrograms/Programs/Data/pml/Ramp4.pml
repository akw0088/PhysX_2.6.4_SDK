<?xml version="1.0"?>
<library id="Ramp4" type="physics">
 <scenedesc source="3dsmax">
  <eye>-30.7277 -71.5279 73.4541</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-30.4419 -70.8547 72.7721</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Sphere01">
   <mass>5.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-15.0998 15.1806 1.00754</position>
   <shape id="shape_Sphere01">
    <mass>5.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.992328</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box01">
   <mass>5.0</mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>17.1521 15.3968 0.0</position>
   <shape id="shape_Box01">
    <mass>5.0</mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.654912</position>
    <geometry id="Box01_geom">
     <box>
      <size>1.30982 1.71285 1.30982</size>
     </box>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box05">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>0.115354 -28.2882 0.0</position>
   <shape id="shape_Box05">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box05_geom">
     <convex facecount="0">
      <vertices count="8">
         -10.3723 -18.0774 0.0,
         10.3723 -18.0774 0.0,
         -10.3723 18.0774 0.0,
         10.3723 18.0774 0.0,
         -10.3723 -9.56609 1.7781,
         10.3723 -9.56609 1.7781,
         -10.3723 7.6618 1.7781,
         10.3723 7.6618 1.7781,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box03">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-0.0582439 30.1887 0.0</position>
   <shape id="shape_Box03">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box03_geom">
     <convex facecount="0">
      <vertices count="8">
         -10.4776 -19.9074 0.0,
         10.4776 -19.9074 0.0,
         -10.4776 19.9074 0.0,
         10.4776 19.9074 0.0,
         -10.4776 -5.2606 1.67641,
         10.4776 -5.2606 1.67641,
         -10.4776 11.3783 1.67641,
         10.4776 11.3783 1.67641,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box02">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-30.0241 0.0132721 0.0</position>
   <shape id="shape_Box02">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box02_geom">
     <convex facecount="0">
      <vertices count="8">
         -19.4883 -10.268 0.0,
         19.4883 -10.268 0.0,
         -19.4883 10.268 0.0,
         19.4883 10.268 0.0,
         -8.45762 -10.2575 2.93372,
         4.85549 -10.268 2.93372,
         -8.45762 10.2785 2.93372,
         4.85549 10.268 2.93372,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Box04">
   <dynamic>0</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>30.5363 0.0132721 0.0</position>
   <shape id="shape_Box04">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box04_geom">
     <convex facecount="0">
      <vertices count="8">
         -19.6978 -9.84892 0.0,
         19.6978 -9.84892 0.0,
         -19.6978 9.84892 0.0,
         19.6978 9.84892 0.0,
         -4.20905 -9.84892 3.35282,
         8.2648 -9.84892 3.35282,
         -4.20905 9.84892 3.35282,
         8.2648 9.84892 3.35282,
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
