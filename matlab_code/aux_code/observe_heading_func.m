function n_p = observe_heading_func(in1)
%OBSERVE_HEADING_FUNC
%    N_P = OBSERVE_HEADING_FUNC(IN1)

%    This function was generated by the Symbolic Math Toolbox version 5.8.
%    27-Jun-2012 13:38:38

q1 = in1(1,:);
q2 = in1(2,:);
q3 = in1(3,:);
q4 = in1(4,:);
n_p = [q1.*q4.*-2.0+q2.*q3.*2.0;q1.^2-q2.^2+q3.^2-q4.^2;q1.*q2.*2.0+q3.*q4.*2.0];
