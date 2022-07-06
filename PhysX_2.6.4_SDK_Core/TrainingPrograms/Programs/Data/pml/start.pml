<?xml version="1.0"?>
<Library id="helloobjects" type="physics">
 <scenedesc source="3dsmax">
  <eye>24.7333 3.72135 12.1139</eye>
  <gravity>0.0 0.0 -10.0</gravity>
  <lookat>23.8533 3.58197 11.66</lookat>
  <up>0.0 0.0 1.0</up>
 </scenedesc>
 <physicsmodel name="groundplanemodel">
  <rigidbody id="groundplaneactor">
   <dynamic>0</dynamic>
   <Mass>0</Mass>
   <shape id="groundplaneshape">
    <geometry id="groundplane">
     <plane>
      <d>0</d>
      <normal>0.0 0.0 1.0</normal>
     </plane>
    </geometry>
   </shape>
  </rigidbody>
 </physicsmodel>
</Library>
