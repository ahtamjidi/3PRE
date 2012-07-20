function hdl_player(varargin)
%HDL_PLAYER  Lidar data playback for Matlab.
%   HDL_PLAYER launches the playback GUI.  The user is prompted to select a
%   libpcap data file.  The first db.xml data file found is used with order
%   of preference given by 1) a db.xml file in the scan data directory or
%   2) a db.xml file in the HDL_PLAYER root directory.  If one cannot be
%   found, the user is prompted to select one.
%
%   HDL_PLAYER PROMPT causes HDL_PLAYER to prompt the user for both the
%   libpcap file *and* the db.xml file.
%
%   (c) 2006 Ryan M. Eustice
%            University of Michigan
%            eustice@umich.edu
%  
%-----------------------------------------------------------------
%    History:
%    Date            Who          What
%    -----------     -------      -----------------------------
%    12-01-2006      RME          Created.
%    12-11-2006      RME          Added GUI file select.
%    02-01-2007      RME          Updated to work with 64-beam data.
%    02-19-2007      RME          Changed sensor frame to be Xs-East, Ys-North, Zs-up
%    07-13-2007      RME          Changed to work in [m] instead of [cm]
%    07-25-2007      RME          Seriously sped up rendering of 3D point cloud intensity.
%    07-26-2007      RME          Modified to use memmap for efficiency.
%    08-07-2007      RME          Modified to use hdl_scan2world.m

N_Laser = [];
persistent Player;
Player.rootPath = fileparts(which(mfilename));
Player.prompt = false;
Player.nLasers = 64;
Player.k = 1;
if ~isfield(Player,'pcapPath')
    Player.pcapPath = Player.rootPath;
end

if isunix
    % workaround for the Matlab bug with uigetfile() on unix/linux systems
    % http://www.mathworks.com/support/bugreports/details.html?rp=259878
    setappdata(0,'UseNativeSystemDialogs',false);
end

if nargin > 0
    switch upper(varargin{1})
        case 'PROMPT'
            Player.prompt = true;
        otherwise
            error('Unknown argument %s.',varargin{1});
    end
end

% load libpcap file
[Player.M, tmp, Player.pcapFile, Player.pcapPath] = hdl_gopenpcap(Player.pcapPath);
if isempty(Player.M)
    uiwait(errordlg('Libpcap file unspecified!','error','modal'));
    return;
end

% load db.xml file
Db = myLoadDb();
if isempty(Db)
    uiwait(errordlg('DSR db.xml file unspecified!','error','modal'));
    return;
end

% load Ping data
Player.gpsinsDir = [Player.pcapPath,'..',filesep,'gpsins'];
if exist(Player.gpsinsDir,'dir')
    Ping = hdl_loadping(Player.gpsinsDir);
else
    Ping = 1;
end

% precache laser geometry
tic;
fprintf('caching laser geom...');
Calib = hdl_lasergeom(Db);
fprintf(' done.\t'); toc;

% pre-parse scan positions within libpcap file
tic;
fprintf('preparsing .pcap file...');
Scan = hdl_fgetscan(Player.M,1,'bof');
Player.nScans = Scan.Meta.K;
fprintf(' done.\t'); toc;

assignin('base','Scan_first',Scan);
% setup figure window for animation
Player.worldFrame = false;
initFigureWindow();   
initGraphicsHandles();
loadScan(1);
renderColorScan();
setAxis('reset');


%**************************************************************************


%=======================================
% nested function: myLoadDb()
%=======================================
    function Db = myLoadDb()
        % prescan path for db.xml file
        %1) search pcapPath data directory
        DirPath = Player.pcapPath;
        DirData = dir(fullfile(Player.pcapPath,'*.xml'));
        if isempty(DirData)
            %2) search rootPath directory
            DirPath = Player.rootPath;
            DirData = dir(fullfile(Player.rootPath,'*.xml'));
        end
        
        % load db.xml file
        dbPathExists = isfield(Player,'dbPath') && ischar(Player.dbPath) && exist(Player.dbPath,'dir');
        if Player.prompt || isempty(DirData)
            if dbPathExists
                cd(Player.dbPath);
            end
            [Player.dbFile,Player.dbPath] = uigetfile(...
                {'*.xml','DSR db.xml Files (*.xml)';...
                 '*.*','All Files (*.*)'},...
                 'Select a DSR db.xml file');
            cd(Player.rootPath);
        else
            Player.dbPath = DirPath;
            Player.dbFile = DirData(1).name;
        end
        tmp = fullfile(Player.dbPath,Player.dbFile);
        if exist(tmp,'file');
            Db = hdl_loaddb(tmp);
        else
            Db = [];
        end
    end % myLoadDb


%=======================================
% nested function: initFigureWindow()
%=======================================
    function initFigureWindow()
        % setup figure window for animation        
        set(0,'showHiddenHandles','on');
        fig = findobj('Type', 'figure', 'Tag', 'hdl_player');
        set(0,'showHiddenHandles','off');
        if ishandle(fig)
            close(fig);
        end
        Player.Hdl.fig = figure(...
            'Name',sprintf('HDL_PLAYER: %s',Player.pcapFile),...
            'NumberTitle','off',...
            'IntegerHandle','off',...
            'Units','normalized',...
            'Position',[0.2 0.1 0.6 0.7],...
            'Visible','on',...
            'Toolbar','figure',...
            'DeleteFcn',@callback_guiClose,...
            'Tag','hdl_player');
        whitebg(Player.Hdl.fig,'black');
        cameratoolbar(Player.Hdl.fig,'show');
        cameratoolbar('SetMode','orbit');

        % setup axis for animation
        Player.Hdl.axe = gca;
        Player.axelimInit = [-100,100,-100,100,-10,10]; %[m]
        set(Player.Hdl.axe,...
            'NextPlot','replaceChildren',...
            'Projection','perspective');
        grid(Player.Hdl.axe,'off');
        axis(Player.Hdl.axe,'xy');
        axis(Player.Hdl.axe,'vis3d');
        set(Player.Hdl.axe,...
            'XTickLabelMode','manual','XTickLabel','',...
            'YTickLabelMode','manual','YTickLabel','',...
            'ZTickLabelMode','manual','ZTickLabel','');
        
        %setup UI control
        Player.Gui.reset = uicontrol(Player.Hdl.fig,...
            'Style','PushButton',...
            'String','Reset',...
            'FontWeight','Bold',...
            'Units','Normalized',...
            'Position',[0.01 0.005 0.075 0.025],...
            'Units','Pixel',...
            'TooltipString','Reset to Nadar view',...
            'Callback',@callback_guiResetView);
        
              
        % BUTTON GROUP LASER COLOR
        %----------------------------------------------------
        Player.Gui.buttonGroupColor = uibuttongroup(...
            'Parent',Player.Hdl.fig,...
            'Units','Normalized',...
            'Position',[0.005 0.05 0.095 0.08],...
            'Units','Pixel',...
            'Visible','off');
        
        Player.Gui.radioLasers = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupColor,...
            'Style','RadioButton',...
            'Units','Normalized',...
            'Position',[0.0 0.05 1.5 0.5],...
            'Interruptible','on',...
            'BusyAction','queue',...
            'String','Laser Index',...
            'TooltipString','Color by laser');
        
        Player.Gui.radioIntensity = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupColor,...
            'Style','RadioButton',...
            'Units','Normalized',...
            'Position',[0.0 0.5 1.5 0.5],...
            'Interruptible','on',...
            'BusyAction','queue',...
            'String','Laser Reflectivity',...
            'TooltipString','Color by reflectivity');

        Player.Gui.radioLaserRange = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupColor,...
            'Style','RadioButton',...
            'Units','Normalized',...
            'Position',[0.0 1.0 1.5 0.5],...
            'Interruptible','on',...
            'BusyAction','queue',...
            'String','Laser Range',...
            'TooltipString','Color by range');     
       
        set(Player.Gui.buttonGroupColor,...
            'SelectionChangeFcn',@callback_guiButtonGroupColor,...
            'SelectedObject',Player.Gui.radioLasers,...
            'Visible','on');
        %----------------------------------------------------
        
        Player.Gui.markerSize = uicontrol(Player.Hdl.fig,...
            'Style','popupMenu',...
            'Units','Normalized',...
            'Position',[0.005 .31 0.095 0.04],...
            'Units','Pixel',...
            'String','Normal|Medium|Large|Huge',...
            'TooltipString','Marker Size',...
            'CallBack',@callback_guiMarkerSize);
        
        % BUTTON GROUP REFERENCE FRAMES
        %----------------------------------------------------
%         Player.Gui.buttonGroupRefFrame = uibuttongroup(...
%             'Parent',Player.Hdl.fig,...
%             'Units','Normalized',...
%             'Position',[0.005 .240 0.095 0.08],...
%             'Units','Pixel',...
%             'Visible','off');
%         
%         Player.Gui.sensorFrame = uicontrol(Player.Hdl.fig,...
%             'Parent',Player.Gui.buttonGroupRefFrame,...
%             'Style','RadioButton',...
%             'Units','Normalized',...
%             'Position',[0.0 0.05 1.0 0.5],...
%             'Interruptible','on',...
%             'BusyAction','queue',...
%             'String','Sensor',...
%             'TooltipString','Display point cloud in sensor-frame.');
        
%         if isempty(Ping)
%             tmp = 'off';
%         else
%             tmp = 'on';
%         end
%         Player.Gui.worldFrame = uicontrol(Player.Hdl.fig,...
%             'Parent',Player.Gui.buttonGroupRefFrame,...
%             'Style','RadioButton',...
%             'Units','Normalized',...
%             'Position',[0.0 0.5 1.0 0.5],...
%             'Interruptible','on',...
%             'BusyAction','queue',...
%             'String','World',...
%             'Enable',tmp,...
%             'TooltipString','Display point cloud in world-frame.');

%         if isempty(Ping)
%             tmp = Player.Gui.sensorFrame; 
%             Player.worldFrame = false;
%         else
%             tmp = Player.Gui.worldFrame;
%             Player.worldFrame = true;
%         end

%         set(Player.Gui.buttonGroupRefFrame,...
%             'SelectionChangeFcn',@callback_guiButtonGroupRefFrame,...
%             'SelectedObject',Player.Gui.sensorFrame,...
%             'Visible','on');
        %----------------------------------------------------
        
        % BUTTON GROUP PLAYBACK
        %----------------------------------------------------
        Player.Gui.buttonGroupPlayback = uibuttongroup(...
            'Parent',Player.Hdl.fig,...
            'Units','Normalized',...
            'Position',[0.1 0.005 0.175 0.04],...
            'Units','Pixel',...
            'Visible','off');
        
        Player.Gui.skipBwd = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupPlayback,...
            'Style','ToggleButton',...
            'Units','Normalized',...
            'Position',[0.0 0.05 0.2 0.9],...
            'Cdata',loadIcon('rewind.png'),...
            'Interruptible','on',...
            'BusyAction','queue',...
            'TooltipString','Play from begining');
        
        Player.Gui.playBwd = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupPlayback,...
            'Style','ToggleButton',...
            'Units','Normalized',...
            'Position',[0.2 0.05 0.2 0.9],...
            'Cdata',loadIcon('play_reverse.png'),...
            'Interruptible','on',...
            'BusyAction','queue',...
            'TooltipString','Play backwards');
        
        Player.Gui.pause = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupPlayback,...
            'Style','ToggleButton',...
            'Units','Normalized',...
            'Position',[0.4 0.05 0.2 0.9],...
            'Cdata',loadIcon('pause.png'),...
            'Interruptible','on',...
            'BusyAction','queue',...
            'TooltipString','Pause');

        Player.Gui.playFwd = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupPlayback,...
            'Style','ToggleButton',...
            'Units','Normalized',...
            'Position',[0.6 0.05 0.2 0.9],...
            'Cdata',loadIcon('play.png'),...
            'Interruptible','on',...
            'BusyAction','queue',...
            'TooltipString','Play forwards');
        
        Player.Gui.skipFwd = uicontrol(Player.Hdl.fig,...
            'Parent',Player.Gui.buttonGroupPlayback,...
            'Style','ToggleButton',...
            'Units','Normalized',...
            'Position',[0.8 0.05 0.2 0.9],...
            'Cdata',loadIcon('ffwd.png'),...
            'Interruptible','on',...
            'BusyAction','queue',...
            'TooltipString','Play from end');
        
        set(Player.Gui.buttonGroupPlayback,...
            'SelectionChangeFcn',@callback_guiButtonGroupPlayback,...
            'SelectedObject',[],...
            'Visible','on');
        %----------------------------------------------------

        Player.Gui.slider = uicontrol(Player.Hdl.fig,...
            'Style','Slider',...
            'String','Scan No.',...
            'Min',1,'Max',Player.nScans,...
            'Value',1,...
            'SliderStep',[1 50]/(Player.nScans-1),...
            'Units','Normalized',...
            'Position',[0.3 0.005 0.4 0.02],...
            'Units','Pixel',...
            'Interruptible','off',...
            'BusyAction','cancel',...
            'UserData',0,...
            'Callback',@callback_guiSlider);
        
        Player.Gui.sliderText = uicontrol(Player.Hdl.fig,...
            'Style','Text',...
            'Units','Normalized',...
            'Position',[0.71 0.005 0.095, 0.025],...
            'Units','Pixel',...
            'TooltipString','Scan number',...
            'String',sprintf('1/%d',Player.nScans));
        
        Player.Gui.export = uicontrol(Player.Hdl.fig,...
            'Style','PushButton',...
            'Units','Normalized',...
            'Position',[0.81 0.005 0.075, 0.025],...
            'Units','Pixel',...
            'String','Export',...
            'Interruptible','off',...
            'BusyAction','cancel',...
            'TooltipString','Export scan data to workspace',...
            'Callback',@callback_guiExport);
        
        Player.Gui.speedUpText = uicontrol(Player.Hdl.fig,...
            'Style','Text',...
            'Units','Normalized',...
            'Position',[0.9 0.005 0.075, 0.04],...
            'Units','Pixel',...            
            'String',sprintf('SpeedUp:\n1'),...
            'TooltipString','Playback speed');
        
        drawnow;
    end % initFigureWindow
    
    %=======================================
    % nested function: initGraphicsHandles()
    %=======================================
    function initGraphicsHandles()
        % initiate lasers
        callback_guiButtonGroupColor(Player.Gui.buttonGroupColor,[],[]);
        callback_guiMarkerSize(Player.Gui.markerSize,[],[]);
        
        % initiate contours
        hold on;
        theta = linspace(0,2*pi,360)';
        c = cos(theta);
        s = sin(theta);
        x = [10*c, 20*c, 30*c, 40*c, 50*c, 60*c, 70*c, 80*c, 90*c, 100*c]; %[m] 
        y = [10*s, 20*s, 30*s, 40*s, 50*s, 60*s, 70*s, 80*s, 90*s, 100*s]; %[m]
        z = repmat(Db.xyz(3)/100,size(x)); %[m]
        Player.Hdl.circles = plot3(x,y,z,'w-');
        hold off;
        
        % draw sensor orb
        hold on;
        Player.Hdl.orb = plot3(0,0,0,'w.');
        set(Player.Hdl.orb,'MarkerSize',15);
        hold off;
        
        % protect GUI by making its handle invisible to gcf or gca
        set(Player.Hdl.fig,'HandleVisibility','callback');
    end % initGraphicsHandles

%=======================================
% nested function: loadScan
%=======================================
    function loadScan(n)
        % fetch scan
        Scan = hdl_fgetscan(Player.M,n,'bof');
%         if Player.worldFrame
%             POSE = evalin('base', 'POSE');
%             %Scan.Pts = hdl_scan2world(Scan,Calib,Ping);
%             [Scan.Pts x_ws J_ws] = scan2world(Scan,Calib,POSE)
%         else % sensor-frame
         Scan.Pts = hdl_scan2sensor(Scan,Calib);
%         end
    end % loadScan

%=======================================
% nested function: renderColorScan
%=======================================
    function renderColorScan()
        if Player.worldFrame
            [x,y,z] = deal('x_w','y_w','z_w');
        else
            [x,y,z] = deal('x_s','y_s','z_s');
        end
        if ishandle(Player.Hdl.fig)
            if (get(Player.Gui.radioLasers,'Value') == 1) % draw laser point cloud colored by laser
                for k = 1:Player.nLasers
                    if ~Db.enabled(k)
                        set(Player.Hdl.lasersAll(k),'Visible','off');
                    else
                        set(Player.Hdl.lasersAll(k),'Visible','on');
                        if (k < 33)
                            set(Player.Hdl.lasersAll(k),...
                                'Xdata', Scan.Pts.(x)(k,Scan.Data.uprInd),...
                                'Ydata', Scan.Pts.(y)(k,Scan.Data.uprInd),...
                                'Zdata', Scan.Pts.(z)(k,Scan.Data.uprInd));
                        else
                            set(Player.Hdl.lasersAll(k),...
                                'Xdata', Scan.Pts.(x)(k-32,Scan.Data.lwrInd),...
                                'Ydata', Scan.Pts.(y)(k-32,Scan.Data.lwrInd),...
                                'Zdata', Scan.Pts.(z)(k-32,Scan.Data.lwrInd));
                        end
                    end
                end
            elseif (get(Player.Gui.radioIntensity,'Value') == 1) % draw laser point cloud colored by reflectivity
                [refc,ii] = sort(Scan.Data.refc(:));
                x = Scan.Pts.(x)(ii);
                y = Scan.Pts.(y)(ii);
                z = Scan.Pts.(z)(ii);
                jj = find(diff(refc));
                uref = refc(jj); % unique reflectivity values
                set(Player.Hdl.lasersAll,'Visible','off');
                for k = 1:length(uref)
                    if k > 1
                        ind = jj(k-1)+1:jj(k);
                    else
                        ind = 1:jj(1);
                    end
                    set(Player.Hdl.lasersAll(uref(k)),'Visible','on');
                    set(Player.Hdl.lasersAll(uref(k)),...
                        'Xdata', x(ind),'Ydata', y(ind),'Zdata', z(ind));
                end
            elseif (get(Player.Gui.radioLaserRange,'Value') == 1)
                %[range_i,ii_i] = sort(Scan.Data.rngc(:));%sort(Scan.Pts.(z));
                x_1 = deal(Scan.Pts.(x)(:));
                y_1 = deal(Scan.Pts.(y)(:));
                z_1 = deal(Scan.Pts.(z)(:));
                
                [range, ii] = sort(z_1(:));
                x = x_1(ii);
                y = y_1(ii);
                z = z_1(ii);

                ii = find(z > -3);
                x = x(ii);
                y = y(ii);
                z = z(ii);

                %jj = find(diff(range));
                jj = find(diff(z));
                len = length(jj)
                urange = range(jj); % unique reflectivity values
                l = length(urange)
                n = floor(l/35);
                lower_bound = -3;
                upper_bound = -2.8;
                %Player.Hdl.lasersAll = plot3(ones(2,1000),ones(2,1000),ones(2,1000),'.');
                set(Player.Hdl.lasersAll,'Visible','off');
                for k = 1:35
%                     if k > 1
%                         ind = find(z < n)
%                         %ind = jj(pt-1)+1:jj(pt + n);
%                         %pt = pt + n;
%                     else
                        ind = find(z < upper_bound & z > lower_bound);
                        lower_bound = upper_bound;
                        upper_bound = upper_bound + 0.2;
                        % ind = 1:jj(n);
                        % pt = k + n;
                    %end
                    set(Player.Hdl.lasersAll(k),'Visible','on');
                    set(Player.Hdl.lasersAll(k),...
                        'Xdata', x(ind),'Ydata', y(ind),'Zdata', z(ind));
                end
                %projectScan(Scan,1);
            end
        end
        setAxis('inFocus');
    end % renderColorScan

%=======================================
% nested function: setAxis
%=======================================
    function setAxis(action)
        switch lower(action)
            case 'reset'
                if Player.worldFrame % world-frame
                    tmp = [Scan.Pts.xi_ws(1), Scan.Pts.xi_ws(1), ...
                           Scan.Pts.xi_ws(2), Scan.Pts.xi_ws(2), ...
                           Scan.Pts.xi_ws(3), Scan.Pts.xi_ws(3)];
                    axis(Player.Hdl.axe,tmp+Player.axelimInit);
                    view(Player.Hdl.axe,90,-90);
                else % sensor-frame
                    axis(Player.Hdl.axe,Player.axelimInit);
                    view(Player.Hdl.axe,0, 90);
                end
            case 'infocus'
                if Player.worldFrame
                    x_ws = Scan.Pts.xi_ws(1:3,end);
                    xlim(Player.Hdl.axe,x_ws(1)+Player.axelimInit(1:2));
                    ylim(Player.Hdl.axe,x_ws(2)+Player.axelimInit(3:4));
                end
            otherwise
                error('Invalid action arg.');
        end
        
    end % setAxis

%=======================================
% nested function: callback_guiClose
%=======================================
    function callback_guiClose(hObject,eventData,Handles)
        set(Player.Gui.pause,'Value',1); % suspend playback
        clear Player.M;
    end % callback_guiClose

%=======================================
% nested function: callback_guiResetView
%=======================================
    function callback_guiResetView(hObject,eventData,Handles)
        cameratoolbar(Player.Hdl.fig,'ResetCamera');
        setAxis('reset');
    end % callback_guiResetView

%=======================================
% nested function: callback_guiSlider
%=======================================
    function callback_guiSlider(hObject,eventData,Handles)
        set(Player.Gui.slider,'UserData',1);
        Player.k = round(get(hObject,'Value'));
        set(Player.Gui.sliderText,...
            'String',sprintf('%d/%d',Player.k,Player.nScans));
        loadScan(Player.k);
        renderColorScan();
        set(Player.Gui.slider,'UserData',0);
    end % callback_guiSlider

%=======================================
% nested function: increment_guiSlider
%=======================================
    function increment_guiSlider(inc)
        waitfor(Player.Gui.slider,'UserData',0)
        Player.k = round(get(Player.Gui.slider,'Value')) + inc;
        Player.k = min(Player.k,Player.nScans);
        Player.k = max(Player.k,1);
        set(Player.Gui.sliderText,...
            'String',sprintf('%d/%d',Player.k,Player.nScans));
        set(Player.Gui.slider,'Value',Player.k);
    end

%=======================================
% nested function: increment_guiExport
%=======================================
    function callback_guiExport(hObject,eventData,Handles)
        % prompt user for range of scans to export
        name = 'Export Scan Data';
        prompt = {'Enter range of scans to export',''};
        numlines = 1;
        defaultAnswer = {num2str(Player.k),num2str(Player.k)};
        answer = inputdlg(prompt,name,numlines,defaultAnswer);
        if isempty(answer)
            return;
        end
        % export scans to user workspace
        scanStart = round(str2double(answer{1}));
        scanEnd = round(str2double(answer{2}));
        nScans = scanEnd-scanStart+1;
        if (scanEnd >= scanStart)
            n=1;
            for k=scanStart:scanEnd
                fprintf('exporting %d/%d...\n',n,nScans);
                % fetch scan
                ScanTmp = hdl_fgetscan(Player.M,k,'bof');
                ScanExport(n).Meta = ScanTmp.Meta; % ScanTmp is used to get around "dissimilar structures error" w/ Pts field
                ScanExport(n).Data = ScanTmp.Data;
                if Player.worldFrame                    
                    ScanExport(n).Pts = hdl_scan2world(ScanExport(n),Calib,Ping);
                else
                    ScanExport(n).Pts = hdl_scan2sensor(ScanExport(n),Calib);
                end
                n=n+1;
            end
            assignin('base','Scan',ScanExport);
            assignin('base','Db',Db);
            assignin('base','Calib',Calib);
            if Player.worldFrame
                assignin('base','Ping',Ping);
                fprintf('Scan, Db, Calib, and Ping exported to workspace.\n');
            else % sensor-frame
                fprintf('Scan, Db, and Calib exported to workspace.\n');
            end
        else
            warning('invalid scan range');
        end
    end % callback_guiExport

%=======================================
% nested function: callback_guiButtonGroupPlayback
%=======================================
    function callback_guiButtonGroupPlayback(source,eventData,Handles)
        hObject = get(source,'SelectedObject');
        switch hObject
            case Player.Gui.pause
                % playback suspended
                hold on;
            case Player.Gui.playBwd
                guiPlayback(hObject,-1);
            case Player.Gui.playFwd
                guiPlayback(hObject,1);
            case Player.Gui.skipFwd
                set(Player.Gui.slider,'Value',Player.nScans);
                set(Player.Gui.playBwd,'Value',1);
                Player.k = Player.nScans;
                guiPlayback(Player.Gui.playBwd,0);
            case Player.Gui.skipBwd
                set(Player.Gui.slider,'Value',1);
                set(Player.Gui.playFwd,'Value',1);
                Player.k = 1;
                guiPlayback(Player.Gui.playFwd,0);
            otherwise
                error('Invalid handle');
        end
    end % callback_guiButtonGroupPlayback


%=======================================
% nested function: callback_guiButtonGroupColor
%=======================================
    function callback_guiButtonGroupColor(source,eventData,Handles)
        if (isfield(Player.Hdl,'lasersAll') && any(ishandle(Player.Hdl.lasersAll)))
            delete(Player.Hdl.lasersAll(ishandle(Player.Hdl.lasersAll)));
        end        
        
        % initiate lasers
        hold on;
        if (get(Player.Gui.radioLasers,'Value') == 1)
            set(Player.Hdl.axe,'ColorOrder',Db.colors);
            Player.Hdl.lasersAll = plot3(ones(2,64),ones(2,64),ones(2,64),'.');
        elseif (get(Player.Gui.radioIntensity,'Value') == 1)
            set(Player.Hdl.axe,'ColorOrder',jet(256));
            Player.Hdl.lasersAll = plot3(ones(2,256),ones(2,256),ones(2,256),'.');
        elseif (get(Player.Gui.radioLaserRange,'Value') == 1)
            set(Player.Hdl.axe,'ColorOrder',hsv(35));
            Player.Hdl.lasersAll = plot3(ones(2,35),ones(2,35),ones(2,35),'.');
        end
        hold off;
        callback_guiMarkerSize(Player.Gui.markerSize,[],[]);
        loadScan(Player.k);
        renderColorScan();

    end % callback_guiButtonGroupColor

%=======================================
% nested function: callback_guiButtonGroupRefFrame
%=======================================
    function callback_guiButtonGroupRefFrame(source,eventData,Handles)
        if get(Player.Gui.sensorFrame,'Value'); % sensor-frame
            Player.worldFrame = false;
        else                                    % world-frame
            Player.worldFrame = true;
        end
        loadScan(Player.k);
        setAxis('reset');
        renderColorScan();
    end % callback_guiButtonGroupRefFrame

%=======================================
% nested function: callback_guiMarkerSize
%=======================================
    function callback_guiMarkerSize(hObject,eventData,Handles)
        switch get(hObject,'value')
            case 1 % normal
                set(Player.Hdl.lasersAll,'MarkerSize',1);
            case 2 % medium
                set(Player.Hdl.lasersAll,'MarkerSize',5);
            case 3 % large
                set(Player.Hdl.lasersAll,'MarkerSize',20);
            case 4 % huge
                set(Player.Hdl.lasersAll,'MarkerSize',50);
            otherwise
                error('Invalid value');
        end
    end

%=======================================
% nested function: guiPlayback
%=======================================
    function guiPlayback(hObject,inc)
        N = 5;
        n = 1;
        speedUp = repmat(nan,[1 N]); % ringbuffer
        while ishandle(Player.Hdl.fig) && (get(hObject,'Value')==1)
            % grab time of prev scan
            unixtime1 = Scan.Data.ts_iunix(1);
            clock1 = clock;
            
            % grab and render scan data
            loadScan(Player.k);
            renderColorScan();
            
            % grab time of current scan
            clock2 = clock;
            unixtime2 = Scan.Data.ts_iunix(1);
            
            % compute elapsed time
            elapsedVirt = etime(clock2,clock1);
            elapsedReal = unixtime2-unixtime1;
            speedUp(n) = elapsedReal/elapsedVirt;
            n = mod(n,N)+1; % increment ringbuffer index
            set(Player.Gui.speedUpText,'String',sprintf('SpeedUp:\n%0.2f',nanmean(speedUp)));
            
            % increment scan number
            increment_guiSlider(inc);

            % store our current plot view
            Player.axelim = axis;
            Player.view = view;

            drawnow;

            if (Player.k == Player.nScans) || (Player.k == 1)
                set(Player.Gui.pause,'Value',1);
                break;
            end
        end;
    end % guiPlayback
end % hdl_player

%=======================================
% subfunction: loadIcon
%=======================================
function g = loadIcon(fname)
[a,map]=imread(['./icons/',fname]);
[r,c,d]=size(a);
g=a;
end