function varargout = hdl_polar(Scan,Db,colorByLaser)
%HDL_POLAR  Lidar calibration visualization tool.
%   HDL_POLAR(SCAN,DB) displays the range and intensity data contained in
%   the SCAN data structure as obtained from HDL_FGETSCAN.  The DB data
%   structure contains data from the 'db.xml' file as loaded into the
%   workspace using HDL_LOADDB.
%
%   HDL_POLAR(SCAN,DB,COLORBYLASER) same as above but if COLORBYLASER is
%   nonzero, the curves are colored by laser (like DSR).
%
%   HDL_POLAR steps through the laser data in geometrical order, i.e., as
%   sorted by laser elevation angle.  The user can manually step through
%   the laser beams, one beam at a time, using the left/right arrow keys.
%   If the user holds down the 'control' key then the step size increases
%   to 5 and if the user holds down the 'shift' key then the step size
%   increases to 10.  An 'autoplay' button is also available.
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    02-22-2007      RME          Created.
%    02-24-2007      RME          Added GUI functionality.
%    06-22-2007      RME          Added color by laser plotting.

if ~exist('colorByLaser','var') || isempty(colorByLaser)
    colorByLaser = 0;
end;

DTOR = pi/180;

% correct the yaw sample angle for each laser's rotCorrection
Scan.Data.yawLas = hdl_yawcorrect(Scan,Db);

% sort lasers by beam elevation from top to bottom
[tmp,kk] = sort(Db.vertCorrection,1,'descend');

% initalize graphics
Player.ii = 1;
Player.refMax = 255;
Player.rngMax = 10;
[Player.Hdl,Player.Gui] = initGraphics();
callback_refMax(Player.Gui.refMax,[],[]);
callback_rngMax(Player.Gui.rngMax,[],[]);

if (nargout > 0)
    varargout{1} = Hdl;
end

%=======================================
% nested function: plotLaser
%=======================================
    function plotLaser(inc)
        Player.ii = Player.ii+inc;
         % wrap around
        if (Player.ii > 64)
            Player.ii = 1;
        elseif (Player.ii < 1)
            Player.ii = 64;
        end
        
        id = Db.id(kk(Player.ii));
        vertCorrection = Db.vertCorrection(kk(Player.ii));
        rotCorrection = Db.rotCorrection(kk(Player.ii));

        if colorByLaser
            color = Db.colors(kk(Player.ii),:);
        else
            color = [1 1 0]; % yellow
        end

        if Player.ii < 33
            blkInd = 'uprInd';
            jj = kk(Player.ii);      % sorted laser index in upper block shot
        else
            blkInd = 'lwrInd';
            jj = kk(Player.ii)-32;   % sorted laser index in lower block shot
        end

        % Db.rotCorrection compensated laser yaw
        yawLas = Scan.Data.yawLas(jj,Scan.Data.(blkInd))*DTOR;
        
        % compute horizontal range from slant range and clip outliers
        slantRng = Scan.Data.rngc(jj,Scan.Data.(blkInd))*Db.distLSB/100; % [m]
        horizRng = slantRng*cos(vertCorrection*DTOR);
        clipRng = horizRng;
        clipRng(clipRng>Player.rngMax) = NaN;
        clipRng(clipRng==0) = NaN;

        % compute reflectivity
        clipRef = Scan.Data.refc(jj,Scan.Data.(blkInd));
        clipRef(clipRef>Player.refMax) = NaN;
        
        % plot title
        str = sprintf('#%d/64   vert: %+0.2f\\circ   rot: %+0.2f\\circ   laserID: %02d',...
                      Player.ii,vertCorrection,rotCorrection,id);
        set(Player.Hdl.title,'String',str);

        % plot reflectivity
        Player.Hdl.ref = polar(Player.Hdl.refAxe,[yawLas,NaN],[clipRef,Player.refMax]);
        set(Player.Hdl.ref,'color',color);
        str = sprintf('Reflectivity [0-255]\nmed = %d\niqr = %d',nanmedian(clipRef),iqr(clipRef));
        ylabel(Player.Hdl.refAxe,str);
        view(Player.Hdl.refAxe,90,-90); % view polar plot oriented to sensor frame

        % plot range
        Player.Hdl.rng = polar(Player.Hdl.rngAxe,[yawLas,NaN],[clipRng,Player.rngMax]);
        set(Player.Hdl.rng,'color',color);
        ylabel(Player.Hdl.rngAxe,'Horizontal Range [meters]');
        view(Player.Hdl.rngAxe,90,-90); % view polar plot oriented to sensor frame

        drawnow;
    end % plotLaser

%=======================================
% nested: callback_rngMax
%=======================================
    function callback_rngMax(hObject,eventData,Handles)
        val = get(hObject,'Value');
        userData = get(hObject,'UserData');
        Player.rngMax = userData(val);
        plotLaser(0)
    end % callback_rngMax

%=======================================
% nested: callback_refMax
%=======================================
    function callback_refMax(hObject,eventData,Handles)
        val = get(hObject,'Value');
        userData = get(hObject,'UserData');
        Player.refMax = userData(val);
        plotLaser(0)
    end % callback_refMax

%=======================================
% nested: callback_playPause
%=======================================
    function callback_playPause(hObject,eventData,Handles)
        val = ~get(hObject,'UserData');
        set(hObject,'UserData',val);
        switch val
            case 0
                set(hObject,'CData',loadIcon('play.png'));
            case 1
                set(hObject,'CData',loadIcon('pause.png'));                
                while ishandle(Player.Hdl.fig) && (get(hObject,'UserData') == val)
                    plotLaser(1);
                    pause(0.5);
                    if (Player.ii == 64)
                        set(hObject,'UserData',~val);
                        set(hObject,'CData',loadIcon('play.png'));
                        break;
                    end
                end
            otherwise error('invalid value');
        end
    end % callback_playPause

%=======================================
% nested: callback_keyPress
%=======================================
    function callback_keyPress(hObject,eventData,Handles)
        if ~isempty(eventData.Modifier)
            switch eventData.Modifier{1}
                case 'control',inc = 5;
                case 'shift', inc = 10;
                otherwise, inc = 1;
            end
        else
            inc = 1;
        end
        switch eventData.Key
            case 'leftarrow'
                plotLaser(-inc);
            case 'rightarrow'
                plotLaser(inc);
            case 'space'
                callback_playPause(Player.Gui.playPause,[],[]);
            otherwise % ignore
        end
    end % callback_keyPress

%=======================================
% nested: initGraphics()
%=======================================
    function [Hdl,Gui] = initGraphics()
        % setup figure window for animation
        set(0,'showHiddenHandles','on');
        fig = findobj('Type', 'figure', 'Tag', 'hdl_polar');
        set(0,'showHiddenHandles','off');
        if ishandle(fig)
            close(fig);
        end
        Hdl.fig = figure(...
            'Name','HDL_POLAR',...
            'NumberTitle','off',...
            'IntegerHandle','off',...
            'Units','normalized',...
            'Position',[0.1 0.375 0.8 0.5],...
            'Visible','on',...
            'Toolbar','figure',...
            'KeyPressFcn',@callback_keyPress,...
            'Tag','hdl_polar');
        whitebg(Hdl.fig,'black');

        % setup graphics
        Hdl.refAxe = subplot(1,2,1);
        set(Hdl.refAxe,'fontSize',14);
        Hdl.rngAxe = subplot(1,2,2);
        set(Hdl.rngAxe,'fontSize',14);
        Hdl.titleAxe = axes('position',[.2 .90 .8 .01]);
        set(Hdl.titleAxe,'fontSize',14);
        set(Hdl.titleAxe,'visible','off');
        Hdl.title = title(Hdl.titleAxe,'Init');
        set(Hdl.title,'visible','on');

        % max range listbox
        val = 5:5:100;
        str = sprintf('%d|',val);
        str = str(1:end-1);
        Gui.rngMax = uicontrol(...
            'Style','popUpMenu',...
            'Units','Normalized',...
            'Position',[0.9 0.9 0.05 0.05],...
            'String',str,...
            'UserData',val,...
            'Value',1,...
            'Callback',@callback_rngMax);

        Gui.rngText = uicontrol(...
            'Style','Text',...
            'Units','Normalized',...
            'Position',[0.9 0.95 0.05 0.025],...
            'String','Max Rng');
        
        % max reflectivity listbox
        val = [20:20:255,255];
        str = sprintf('%d|',val);
        str = str(1:end-1);
        Gui.refMax = uicontrol(...
            'Style','popUpMenu',...
            'Units','Normalized',...
            'Position',[0.1 0.9 0.05 0.05],...
            'String',str,...
            'UserData',val,...
            'Value',length(val),...
            'Callback',@callback_refMax);

        Gui.refText = uicontrol(...
            'Style','Text',...
            'Units','Normalized',...
            'Position',[0.1 0.95 0.05 0.025],...
            'String','Max Ref');

        % autoplay
        str = 'Use the left/right arrows OR press play to auto step through the laser sequence.';
        Gui.playPause = uicontrol(...
            'Style','Pushbutton',...
            'Units','Normalized',...
            'Position',[0.5 0.1 0.025 0.05],...
            'Units','Pixel',...
            'UserData',0,...
            'CData',loadIcon('play.png'),...
            'Interruptible','on',...
            'BusyAction','cancel',...
            'Tooltip',str,...
            'Callback',@callback_playPause);
        
        Gui.playPauseText = uicontrol(...
            'Style','Text',...
            'Units','Normalized',...
            'Position',[0.49 0.15 0.05 0.03],...
            'String','AutoPlay');
        
        % protect GUI by making its handle invisible to gcf or gca
        set(Hdl.fig,'HandleVisibility','callback');
    end % initGraphics

end % hdl_polar

%=======================================
% subfunction: loadIcon
%=======================================
function g = loadIcon(fname)
[a,map]=imread(['./icons/',fname]);
[r,c,d]=size(a);
g=a;
end
