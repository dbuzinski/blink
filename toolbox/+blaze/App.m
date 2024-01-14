classdef App
    %UNTITLED Summary of this class goes here
    %   Detailed explanation goes here

    properties
        Port (1,1) double
        Route function_handle
    end

    methods
        function app = App(options)
            arguments
                options.Port (1,1) double = 3000;
                options.Route function_handle = @() "";
            end
            app.Port = options.Port;
            app.Route = options.Route;
        end

        function serve(app)
            blaze.internal.serve(app);
        end
    end
end