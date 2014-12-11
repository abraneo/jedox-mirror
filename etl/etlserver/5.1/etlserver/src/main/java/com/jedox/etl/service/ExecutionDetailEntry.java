package com.jedox.etl.service;

public class ExecutionDetailEntry {
	
	private String locator;
	private Long runtime;
	private Long processedInputRows;
	private Long processedOutputRows;
	private String type;
	private String scope;
	private Integer inputCalls;
	private Integer outputCalls;
	
	public void setLocator(String locator) {
		this.locator = locator;
	}
	public String getLocator() {
		return locator;
	}
	public void setRuntime(Long runtime) {
		this.runtime = runtime;
	}
	public Long getRuntime() {
		return runtime;
	}
	public void setProcessedInputRows(Long processedInputRows) {
		this.processedInputRows = processedInputRows;
	}
	public Long getProcessedInputRows() {
		return processedInputRows;
	}
	public void setProcessedOutputRows(Long processedOutputRows) {
		this.processedOutputRows = processedOutputRows;
	}
	public Long getProcessedOutputRows() {
		return processedOutputRows;
	}
	public void setType(String type) {
		this.type = type;
	}
	public String getType() {
		return type;
	}
	public void setScope(String scope) {
		this.scope = scope;
	}
	public String getScope() {
		return scope;
	}
	public void setInputCalls(Integer inputCalls) {
		this.inputCalls = inputCalls;
	}
	public Integer getInputCalls() {
		return inputCalls;
	}
	public void setOutputCalls(Integer outputCalls) {
		this.outputCalls = outputCalls;
	}
	public Integer getOutputCalls() {
		return outputCalls;
	}

}
