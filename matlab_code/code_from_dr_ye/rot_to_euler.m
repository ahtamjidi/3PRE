function [phi, theta, psi] = rot_to_euler(rot)

psi = atan2(-rot(1,2), rot(2, 2));
theta = atan2(rot(3,2), -rot(1,2)*sin(psi)+rot(2,2)*cos(psi));
phi = atan2(rot(1,3)*cos(psi)+rot(2,3)*sin(psi), rot(1,1)*cos(psi)+rot(2,1)*sin(psi));