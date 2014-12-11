package com.jedox.etl.core.execution;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;

import com.jedox.etl.core.component.ITypes;
import com.jedox.etl.core.persistence.hibernate.IPersistable;

@Entity
public class ExecutionDetail implements IPersistable {
	
	@Id @GeneratedValue(strategy=GenerationType.TABLE)
	@Column(name = "MESSAGE_ID") 
	private Long id;
	private Long resultId;
	@Column(length = 1000) 
	private String locator;
	@Column(length = 1000) 
	private String type;
	@Enumerated(EnumType.ORDINAL) 
	private ITypes.MainManagers scope;
	private Long runtime = new Long(0);
	private Long processedInputRows = new Long(0);
	private Long processedOutputRows = new Long(0);
	private Integer inputCalls = 0;
	private Integer outputCalls = 0;

	@Override
	public Long getId() {
		return id;
	}
	
	public void setId(Long id) {
		this.id = id;
	}

	public void setResultId(Long resultId) {
		this.resultId = resultId;
	}

	public Long getResultId() {
		return resultId;
	}

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

	public void setScope(ITypes.MainManagers scope) {
		this.scope = scope;
	}

	public ITypes.MainManagers getScope() {
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
