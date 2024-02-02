
int normalToPixel[16] = { 1, 3, 3, 7, 7, 5, 5, 1, 0, 6, 6, 8, 8, 2, 2, 0 };
float normalWeights[8] = { 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.5 };

vec3 computeNormal_perspective(in float[9] depth)
{
   if (depth[4] >= pc.nearFar.y - 1.0)
      return vec3(0.0, 0.0, 0.0);

   vec3 c0 = vec3((gl_GlobalInvocationID.x + pc.screenOffset.x) * pc.pxSize.x, (gl_GlobalInvocationID.y + pc.screenOffset.y) * pc.pxSize.y, 1.0);

   vec3 p4 = c0 * depth[4];
   vec3 p1 = (c0 + vec3(-pc.pxSize.x, 0, 0)) * depth[1];
   vec3 p3 = (c0 + vec3(0, -pc.pxSize.y, 0)) * depth[3];
   vec3 p5 = (c0 + vec3(0, pc.pxSize.y, 0)) * depth[5];
   vec3 p7 = (c0 + vec3(pc.pxSize.x, 0, 0)) * depth[7];
   vec3 p0 = (c0 + vec3(-pc.pxSize.x, -pc.pxSize.y, 0)) * depth[0];
   vec3 p2 = (c0 + vec3(-pc.pxSize.x, pc.pxSize.y, 0)) * depth[2];
   vec3 p6 = (c0 + vec3(pc.pxSize.x, -pc.pxSize.y, 0)) * depth[6];
   vec3 p8 = (c0 + vec3(pc.pxSize.x, pc.pxSize.y, 0)) * depth[8];

   vec3 v41 = p1 - p4;
   vec3 v43 = p3 - p4;
   vec3 v45 = p5 - p4;
   vec3 v47 = p7 - p4;
   vec3 v40 = p0 - p4;
   vec3 v42 = p2 - p4;
   vec3 v46 = p6 - p4;
   vec3 v48 = p8 - p4;

   vec3 n[8];
   n[0] = cross(v41, v43);
   n[1] = cross(v43, v47);
   n[2] = cross(v47, v45);
   n[3] = cross(v45, v41);
   n[4] = cross(v40, v46);
   n[5] = cross(v46, v48);
   n[6] = cross(v48, v42);
   n[7] = cross(v42, v40);

   vec3 meanN = vec3(0.0, 0.0, 0.0);
   float weight = 0.0;
   for (int i = 0; i < 8; i++)
   {
      if (depth[normalToPixel[2*i]] < pc.nearFar.y &&
          depth[normalToPixel[2*i+1]] < pc.nearFar.y)
      {
         meanN += n[i] / length(n[i]) * normalWeights[i];
         weight += normalWeights[i];
      }
   }

   return (weight > 0.0) ? (meanN / weight) : vec3(1.0, 1.0, 1.0);
}

vec3 computeNormal_ortho(in float[9] depth)
{
   if (depth[4] >= pc.nearFar.y - 1.0)
      return vec3(0.0, 0.0, 0.0);

   vec3 v41 = vec3(-pc.pxSize.x, 0, depth[1] - depth[4]);
   vec3 v43 = vec3(0, -pc.pxSize.y, depth[3] - depth[4]);
   vec3 v45 = vec3(0, pc.pxSize.y, depth[5] - depth[4]);
   vec3 v47 = vec3(pc.pxSize.x, 0, depth[7] - depth[4]);
   vec3 v40 = vec3(-pc.pxSize.x, -pc.pxSize.y, depth[0] - depth[4]);
   vec3 v42 = vec3(-pc.pxSize.x, pc.pxSize.y, depth[2] - depth[4]);
   vec3 v46 = vec3(pc.pxSize.x, -pc.pxSize.y, depth[6] - depth[4]);
   vec3 v48 = vec3(pc.pxSize.x, pc.pxSize.y, depth[8] - depth[4]);

   vec3 n[8];
   n[0] = cross(v41, v43);
   n[1] = cross(v43, v47);
   n[2] = cross(v47, v45);
   n[3] = cross(v45, v41);
   n[4] = cross(v40, v46);
   n[5] = cross(v46, v48);
   n[6] = cross(v48, v42);
   n[7] = cross(v42, v40);

   vec3 meanN = vec3(0.0, 0.0, 0.0);
   float weight = 0.0;
   for (int i = 0; i < 8; i++)
   {
      if (depth[normalToPixel[2*i]] < pc.nearFar.y &&
          depth[normalToPixel[2*i+1]] < pc.nearFar.y)
      {
         meanN += n[i] / length(n[i]) * normalWeights[i];
         weight += normalWeights[i];
      }
   }

   return (weight > 0.0) ? (meanN / weight) : vec3(1.0, 1.0, 1.0);
}
