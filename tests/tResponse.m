classdef tResponse < matlab.unittest.TestCase
    methods(Test)
        function testResponseCreation(testCase)
            resp = blink.Response();
            
            testCase.verifyEqual(resp.StatusCode, 200);
            testCase.verifyEqual(resp.Data, "");
        end
    end
end
