classdef tApp < matlab.unittest.TestCase
    methods(Test)
        function testRouteRegistration(testCase)
            app = blink.App();
            
            app.get("/test", @(req, resp) resp);
            app.post("/test", @(req, resp) resp);
            app.put("/test", @(req, resp) resp);
            app.delete("/test", @(req, resp) resp);
            
            testCase.verifyEqual(length(app.Routes), 4);
            testCase.verifyEqual(app.Routes(1).HttpMethod, "GET");
            testCase.verifyEqual(app.Routes(2).HttpMethod, "POST");
            testCase.verifyEqual(app.Routes(3).HttpMethod, "PUT");
            testCase.verifyEqual(app.Routes(4).HttpMethod, "DELETE");
        end
        
        function testInvalidHttpMethod(testCase)
            testCase.verifyError(@() blink.Route("INVALID", "/test", @(req, resp) resp), ...
                'MATLAB:validators:mustBeMember');
        end
        
        function testHandlerFunctionValidation(testCase)
            app = blink.App();
            testCase.verifyError(@() app.get("/test", "not_a_function"), ...
                'MATLAB:validation:UnableToConvert');
        end
    end
end
