classdef tRoute < matlab.unittest.TestCase
    methods(Test)
        function testRouteCreation(testCase)
            handler = @(req, resp) resp;
            route = blink.Route('GET', '/test', handler);
            
            testCase.verifyEqual(route.HttpMethod, 'GET');
            testCase.verifyEqual(route.Path, '/test');
            testCase.verifyEqual(route.Handler, handler);
        end
        
        function testInvalidHttpMethod(testCase)
            handler = @(req, resp) resp;
            
            testCase.verifyError(@() blink.Route('INVALID', '/test', handler), ...
                'MATLAB:validators:mustBeMember');
            testCase.verifyError(@() blink.Route('', '/test', handler), ...
                'MATLAB:validators:mustBeMember');
        end
        
        function testHandlerValidation(testCase)
            testCase.verifyError(@() blink.Route('GET', '/test', 'not_a_function'), ...
                'MATLAB:validation:UnableToConvert');
            testCase.verifyError(@() blink.Route('GET', '/test', 123), ...
                'MATLAB:validation:UnableToConvert');
        end
    end
end
