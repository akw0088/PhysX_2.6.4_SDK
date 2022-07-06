<?xml version="1.0"?>
<library id="catapult" type="physics">
 <scenedesc source="3dsmax">
  <eye>-3.6293 0.223681 1.67711</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>-2.72076 0.12819 1.27037</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Pyramid01">
   <Mass>1000.0</Mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-0.00152762 -0.0105212 5e-005</position>
   <shape>
    <Mass>1000.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Pyramid01_geom">
     <convex facecount="0">
      <vertices count="6">
         0.0 0.0 0.316387,
         -0.118229 -0.0982465 0.0,
         0.118229 -0.0982465 0.0,
         0.118229 0.0982465 0.0,
         -0.118229 0.0982465 0.0,
         0.0 0.0 0.0,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="arm">
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>-0.00526163 -0.198467 0.347816</position>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>-4.48599e-005 0.0633714 -0.0102321</position>
    <geometry id="Box01_geom">
     <box>
      <size>0.0476576 0.835714 0.0445938</size>
     </box>
    </geometry>
   </shape>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.112998 -0.47965 0.0284639</position>
    <geometry id="Box04_geom">
     <convex facecount="0">
      <vertices count="8">
         -0.0416299 -0.111568 -0.0619182,
         0.0416299 -0.111568 0.0,
         -0.0416299 0.111568 -0.0619182,
         0.0416299 0.111568 0.0,
         -0.0416299 -0.111568 0.00335735,
         0.0416299 -0.111568 0.0652756,
         -0.0416299 0.111568 0.00335735,
         0.0416299 0.111568 0.0652756,
      </vertices>
     </convex>
    </geometry>
   </shape>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 -0.707107 0.707107</orientation>
    <position>0.000886927 -0.550264 0.0321317</position>
    <geometry id="Box06_geom">
     <convex facecount="0">
      <vertices count="8">
         -0.0416299 -0.111568 -0.0619182,
         0.0416299 -0.111568 0.0,
         -0.0416299 0.111568 -0.0619182,
         0.0416299 0.111568 0.0,
         -0.0416299 -0.111568 0.00335735,
         0.0416299 -0.111568 0.0652756,
         -0.0416299 0.111568 0.00335735,
         0.0416299 0.111568 0.0652756,
      </vertices>
     </convex>
    </geometry>
   </shape>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 1.0 0.0</orientation>
    <position>-0.112998 -0.477291 0.0284639</position>
    <geometry id="Box03_geom">
     <convex facecount="0">
      <vertices count="8">
         -0.0416299 -0.111568 -0.0619182,
         0.0416299 -0.111568 0.0,
         -0.0416299 0.111568 -0.0619182,
         0.0416299 0.111568 0.0,
         -0.0416299 -0.111568 0.00335735,
         0.0416299 -0.111568 0.0652756,
         -0.0416299 0.111568 0.00335735,
         0.0416299 0.111568 0.0652756,
      </vertices>
     </convex>
    </geometry>
   </shape>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.00476036 -0.47965 0.000774901</position>
    <geometry id="Box02_geom">
     <box>
      <size>0.299735 0.229797 0.0666079</size>
     </box>
    </geometry>
   </shape>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 0.707107 0.707107</orientation>
    <position>0.00746651 -0.408295 0.0321317</position>
    <geometry id="Box05_geom">
     <convex facecount="0">
      <vertices count="8">
         -0.0416299 -0.111568 -0.0619182,
         0.0416299 -0.111568 0.0,
         -0.0416299 0.111568 -0.0619182,
         0.0416299 0.111568 0.0,
         -0.0416299 -0.111568 0.00335735,
         0.0416299 -0.111568 0.0652756,
         -0.0416299 0.111568 0.00335735,
         0.0416299 0.111568 0.0652756,
      </vertices>
     </convex>
    </geometry>
   </shape>
   <shape>
    <Mass>100.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>-0.00506769 0.52967 -0.032529</position>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.0648782</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Sphere02">
   <Mass>1.0</Mass>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>0.0277286 -0.673652 0.468108</position>
   <shape>
    <Mass>1.0</Mass>
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Sphere02_geom">
     <sphere>
      <radius>0.0689729</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <joint child="#arm" class="Reactor:Hinge" id="Hinge01joint" parent="#Pyramid01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.190737 0.0</offset>
    <x>-0.00332563 0.0 0.999994</x>
    <y>0.0 1.0 0.0</y>
    <z>-0.999994 0.0 -0.00332558</z>
   </childplacement>
   <limitmax>0.0 0.0 72.9729</limitmax>
   <limitmin>0.0 0.0 -51.3</limitmin>
   <parentplacement frame="parent" setforbody="parent">
    <offset>-0.00373401 0.00279154 0.347766</offset>
    <x>-0.00332563 0.0 0.999994</x>
    <y>0.0 1.0 0.0</y>
    <z>-0.999994 0.0 -0.00332558</z>
   </parentplacement>
   <Spring>0</Spring>
  </joint>
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
