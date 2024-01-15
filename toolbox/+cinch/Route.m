classdef Route
    properties
        HttpMethod (1,1) cinch.HttpMethod
        Path (1,:) char
        Handler function_handle {mustBeScalarOrEmpty}
    end

    methods
        function route = Route(method, path, handler)
            arguments
                method (1,1) cinch.HttpMethod
                path (1,:) char
                handler (1,1) function_handle
            end
            route.HttpMethod = method;
            route.Path = path;
            route.Handler = handler;
        end
    end
end