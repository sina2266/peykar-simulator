% [f, t, ps] = mexFT_Step(2, [-2.5 2.5], 32, 0.0);
% for i=1:50
%     plot(ps(:,1), ps(:,2), '.');
%     pause(0.5);
%     [t, ps] = mexFT_Step(f, 0.01*i);
% end
% mexFT_Free(f);
[f, t, ps] = mexFT_Step(2, [-2.5 2.5], [-2.5 2.5], 100, 0.0);
dt = 0.1;
T_max = 1;
for i=1:round(T_max/dt)
    plot(ps(:,1), ps(:,2), '.');
    fi = 0:0.00001:2*pi;
    hold on
    plot(cos(fi)+dt*(i-1), sin(fi),'k')
    hold off
    pause(0.01);
    [t, ps] = mexFT_Step(f, dt*i);
end
mexFT_Free(f);