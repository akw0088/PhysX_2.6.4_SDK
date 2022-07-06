<?xml version="1.0"?>
<library id="ragman_completed" type="physics">
 <scenedesc source="3dsmax">
  <eye>0.433523 -2.27099 1.07174</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <groundplane>0.0 0.0 1.0 0.0</groundplane>
  <lookat>0.312926 -1.2968 0.880943</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel id="main">
  <rigidbody id="Box01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>0.0 0.0 0.4</position>
   <shape id="shape_Box01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Box01_geom">
     <convex facecount="0">
      <vertices count="80">
         -0.0991004 -0.0406915 0.0158563,
         -0.0353052 -0.0434899 0.00462963,
         0.0353052 -0.0434899 0.00462963,
         0.0991004 -0.0406915 0.0158563,
         -0.100076 -0.0136974 0.0142488,
         -0.0357899 -0.0146957 0.00223404,
         0.0357899 -0.0146957 0.00223404,
         0.100076 -0.0136974 0.0142488,
         -0.100076 0.0136974 0.0142488,
         -0.0357899 0.0146957 0.00223404,
         0.0357899 0.0146957 0.00223404,
         0.100076 0.0136974 0.0142488,
         -0.0991004 0.0406915 0.0158563,
         -0.0353052 0.0434899 0.00462963,
         0.0353052 0.0434899 0.00462963,
         0.0991004 0.0406915 0.0158563,
         -0.0991004 -0.0406915 0.342355,
         -0.0353052 -0.0434899 0.353581,
         0.0353052 -0.0434899 0.353581,
         0.0991004 -0.0406915 0.342355,
         -0.100076 -0.0136974 0.343962,
         -0.0357899 -0.0146957 0.355977,
         0.0357899 -0.0146957 0.355977,
         0.100076 -0.0136974 0.343962,
         -0.100076 0.0136974 0.343962,
         -0.0357899 0.0146957 0.355977,
         0.0357899 0.0146957 0.355977,
         0.100076 0.0136974 0.343962,
         -0.0991004 0.0406915 0.342355,
         -0.0353052 0.0434899 0.353581,
         0.0353052 0.0434899 0.353581,
         0.0991004 0.0406915 0.342355,
         -0.116043 -0.0476485 0.0644097,
         -0.0454307 -0.0559628 0.0443962,
         0.0454307 -0.0559628 0.0443962,
         0.116043 -0.0476485 0.0644097,
         0.118478 -0.016216 0.0620036,
         0.118478 0.016216 0.0620036,
         0.116043 0.0476485 0.0644097,
         0.0454307 0.0559628 0.0443962,
         -0.0454307 0.0559628 0.0443962,
         -0.116043 0.0476485 0.0644097,
         -0.118478 0.016216 0.0620036,
         -0.118478 -0.016216 0.0620036,
         -0.135596 -0.0556769 0.134432,
         -0.0684108 -0.0842703 0.111489,
         0.0684108 -0.0842703 0.111489,
         0.135596 -0.0556769 0.134432,
         0.141107 -0.0193133 0.132616,
         0.141107 0.0193133 0.132616,
         0.135596 0.0556769 0.134432,
         0.0684108 0.0842703 0.111489,
         -0.0684108 0.0842703 0.111489,
         -0.135596 0.0556769 0.134432,
         -0.141107 0.0193133 0.132616,
         -0.141107 -0.0193133 0.132616,
         -0.135596 -0.0556769 0.223779,
         -0.0684108 -0.0842703 0.246722,
         0.0684108 -0.0842703 0.246722,
         0.135596 -0.0556769 0.223779,
         0.141107 -0.0193133 0.225595,
         0.141107 0.0193133 0.225595,
         0.135596 0.0556769 0.223779,
         0.0684108 0.0842703 0.246722,
         -0.0684108 0.0842703 0.246722,
         -0.135596 0.0556769 0.223779,
         -0.141107 0.0193133 0.225595,
         -0.141107 -0.0193133 0.225595,
         -0.116043 -0.0476485 0.293801,
         -0.0454307 -0.0559628 0.313815,
         0.0454307 -0.0559628 0.313815,
         0.116043 -0.0476485 0.293801,
         0.118478 -0.016216 0.296207,
         0.118478 0.016216 0.296207,
         0.116043 0.0476485 0.293801,
         0.0454307 0.0559628 0.313815,
         -0.0454307 0.0559628 0.313815,
         -0.116043 0.0476485 0.293801,
         -0.118478 0.016216 0.296207,
         -0.118478 -0.016216 0.296207,
      </vertices>
     </convex>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule01">
   <dynamic>1</dynamic>
   <orientation>0.0 -0.707107 0.0 0.707107</orientation>
   <position>-0.12 0.0 0.68</position>
   <shape id="shape_Capsule01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule01_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule04">
   <dynamic>1</dynamic>
   <orientation>0.0 -0.707107 0.0 0.707107</orientation>
   <position>-0.32 0.0 0.68</position>
   <shape id="shape_Capsule04">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule04_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule02">
   <dynamic>1</dynamic>
   <orientation>0.0 0.707107 0.0 0.707107</orientation>
   <position>0.12 0.0 0.68</position>
   <shape id="shape_Capsule02">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule02_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule03">
   <dynamic>1</dynamic>
   <orientation>0.0 0.707107 0.0 0.707107</orientation>
   <position>0.32 0.0 0.68</position>
   <shape id="shape_Capsule03">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule03_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule05">
   <dynamic>1</dynamic>
   <orientation>0.0 0.991508 0.0 -0.130047</orientation>
   <position>-0.06 0.0 0.427331</position>
   <shape id="shape_Capsule05">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule05_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule08">
   <dynamic>1</dynamic>
   <orientation>0.0 -0.991508 0.0 0.130047</orientation>
   <position>-0.118626 0.00710448 0.207689</position>
   <shape id="shape_Capsule08">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule08_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule06">
   <dynamic>1</dynamic>
   <orientation>0.0 0.990563 0.0 0.137056</orientation>
   <position>0.06 0.0 0.427331</position>
   <shape id="shape_Capsule06">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule06_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Capsule07">
   <dynamic>1</dynamic>
   <orientation>0.0 -0.990563 0.0 -0.137056</orientation>
   <position>0.121726 0.00710448 0.208541</position>
   <shape id="shape_Capsule07">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <geometry id="Capsule07_geom">
     <capsule>
      <p0>0.0 0.0 0.05</p0>
      <p1>0.0 0.0 0.2</p1>
      <radius>0.05</radius>
     </capsule>
    </geometry>
   </shape>
  </rigidbody>
  <rigidbody id="Sphere01">
   <dynamic>1</dynamic>
   <orientation>0.0 0.0 0.0 1.0</orientation>
   <position>0.0 0.0 0.757022</position>
   <shape id="shape_Sphere01">
    <orientation>0.0 0.0 0.0 1.0</orientation>
    <position>0.0 0.0 0.066958</position>
    <geometry id="Sphere01_geom">
     <sphere>
      <radius>0.0856</radius>
     </sphere>
    </geometry>
   </shape>
  </rigidbody>
  <joint child="#Capsule01" class="novodex" id="Joint_Capsule01" parent="#Box01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.0 0.0 -1.0</x>
    <y>0.0 1.0 0.0</y>
    <z>1.0 0.0 0.0</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>-0.12 0.0 0.28</offset>
    <x>1.0 0.0 0.0</x>
    <y>0.0 1.0 0.0</y>
    <z>0.0 0.0 1.0</z>
   </parentplacement>
   <rotlimitmax>34.0913 31.1239 33.0338</rotlimitmax>
   <rotlimitmin>-34.0913 -31.1239 -28.1232</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule04" class="novodex" id="Joint_Capsule04" parent="#Capsule01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.0 0.0 0.2</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 0.0</rotlimitmax>
   <rotlimitmin>0.0 0.0 -93.2778</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule02" class="novodex" id="Joint_Capsule02" parent="#Box01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.0 0.0 1.0</x>
    <y>0.0 1.0 0.0</y>
    <z>-1.0 0.0 0.0</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.12 0.0 0.28</offset>
    <x>1.0 0.0 0.0</x>
    <y>0.0 1.0 0.0</y>
    <z>0.0 0.0 1.0</z>
   </parentplacement>
   <rotlimitmax>39.4532 29.5784 37.26</rotlimitmax>
   <rotlimitmin>-39.4532 -29.5784 -25.9656</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule03" class="novodex" id="Joint_Capsule03" parent="#Capsule02">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.0 0.0 0.2</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 0.0</rotlimitmax>
   <rotlimitmin>0.0 0.0 -88.9582</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule05" class="novodex" id="Joint_Capsule05" parent="#Box01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>-0.966175 0.0 -0.257886</x>
    <y>0.0 1.0 0.0</y>
    <z>0.257886 0.0 -0.966175</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>-0.06 0.0 0.0273314</offset>
    <x>1.0 0.0 0.0</x>
    <y>0.0 1.0 0.0</y>
    <z>0.0 0.0 1.0</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 360.0</rotlimitmax>
   <rotlimitmin>0.0 0.0 -360.0</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule08" class="novodex" id="Joint_Capsule08" parent="#Capsule05">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>-0.257886 0.0 0.966175</x>
    <y>0.0 -0.999999 0.0</y>
    <z>0.966175 0.0 0.257886</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.0 0.00710448 0.227331</offset>
    <x>-0.257886 0.0 0.966175</x>
    <y>0.0 -0.999999 0.0</y>
    <z>0.966175 0.0 0.257886</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 0.0656106</rotlimitmax>
   <rotlimitmin>0.0 0.0 -87.2116</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule06" class="novodex" id="Joint_Capsule06" parent="#Box01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>-0.962431 0.0 0.271526</x>
    <y>0.0 1.0 0.0</y>
    <z>-0.271526 0.0 -0.962431</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.06 0.0 0.0273314</offset>
    <x>1.0 0.0 0.0</x>
    <y>0.0 1.0 0.0</y>
    <z>0.0 0.0 1.0</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 360.0</rotlimitmax>
   <rotlimitmin>0.0 0.0 -360.0</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Capsule07" class="novodex" id="Joint_Capsule07" parent="#Capsule06">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.271526 0.0 0.962431</x>
    <y>0.0 -1.0 0.0</y>
    <z>0.962431 0.0 -0.271526</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.0 0.00710448 0.227331</offset>
    <x>0.271526 0.0 0.962431</x>
    <y>0.0 -1.0 0.0</y>
    <z>0.962431 0.0 -0.271526</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 0.034264</rotlimitmax>
   <rotlimitmin>0.0 0.0 -95.4065</rotlimitmin>
   <Spring>0</Spring>
  </joint>
  <joint child="#Sphere01" class="novodex" id="Joint_Sphere01" parent="#Box01">
   <bodycollide>0</bodycollide>
   <childplacement frame="child" setforbody="child">
    <offset>0.0 0.0 0.0</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </childplacement>
   <parentplacement frame="parent" setforbody="parent">
    <offset>0.0 0.0 0.357022</offset>
    <x>0.0 1.0 0.0</x>
    <y>0.0 0.0 -1.0</y>
    <z>-1.0 0.0 0.0</z>
   </parentplacement>
   <rotlimitmax>0.0 0.0 22.6838</rotlimitmax>
   <rotlimitmin>0.0 0.0 -48.7575</rotlimitmin>
   <Spring>0</Spring>
  </joint>
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
