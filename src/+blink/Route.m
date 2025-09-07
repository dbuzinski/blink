classdef Route
    properties
        HttpMethod (1,:) char
        Path (1,:) char
        Handler function_handle {mustBeScalarOrEmpty}
    end

    methods
        function route = Route(method, path, handler)
            arguments
                method (1,:) char {mustBeMember(method, {'GET','POST','PUT', 'DELETE'})}
                path (1,:) char
                handler (1,1) function_handle
            end
            route.HttpMethod = method;
            route.Path = path;
            route.Handler = handler;
        end
    end
end