% Batch computation of euler angle and translation for statistical analysis
%********************************************************************
for j=1:200
    [phi(j), theta(j), psi(j), trans(:,j)] =vot(j);
end
r2d=180.0/pi;

tr = [phi*r2d; theta*r2d; psi*r2d; trans];
fid=fopen('of2\statistics_res_3.dat', 'w');
fprintf(fid, '%12.8f %12.8f %12.8f %12.8f %12.8f %12.8f\n', tr);
fclose(fid);
%********************************************************************

fp=fopen('of2\statistics_res_fw3.dat','r');
dd=fscanf(fp, '%f', [6, 1999]);
fclose(fp);

r2d=180/pi;
for i=1:3
    dd(i,:)=dd(i,:)*r2d;
end

dmin=min(dd, [], 2);
dmax=max(dd, [], 2);

for i=1:6
    du(i,:)=(dmax(i)-dmin(i))/100.0;
    bin(i,:)=dmin(i):du(i,:):dmax(i);
    fg(i)=figure(i);
    h(i,:)=hist(dd(i,:), bin(i,:));
    bar(bin(i,:), h(i,:));
    grid on;
end

figure(fg(1));  title('Roll (\phi) error distribution');  xlabel('\phi (degree)');  ylabel('Number');
figure(fg(2));  title('Pitch (\theta) error distribution');  xlabel('\theta (degree)');  ylabel('Number');
figure(fg(3));  title('Yaw (\psi) error distribution');  xlabel('\psi (degree)');  ylabel('Number');
figure(fg(4));  title('X error distribution'); xlabel('X (meter)');  ylabel('Number');
figure(fg(5));  title('Y error distribution'); xlabel('Y (meter)');  ylabel('Number');
figure(fg(6));  title('Z error distribution'); xlabel('Z (meter)');  ylabel('Number');
